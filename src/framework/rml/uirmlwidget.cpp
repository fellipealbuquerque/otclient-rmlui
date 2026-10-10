/*
 * Copyright (c) 2010-2026 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "uirmlwidget.h"
#include "rmlmanager.h"

#include <framework/graphics/drawpoolmanager.h>
#include <framework/otml/otmlnode.h>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

namespace
{
    // The debugger can only be attached to one context at a time.
    UIRmlWidget* g_debuggerOwner = nullptr;
    bool g_debuggerInitialised = false;

    int toRmlModifiers(const int modifiers)
    {
        int state = 0;
        if (modifiers & Fw::KeyboardCtrlModifier) state |= Rml::Input::KM_CTRL;
        if (modifiers & Fw::KeyboardShiftModifier) state |= Rml::Input::KM_SHIFT;
        if (modifiers & Fw::KeyboardAltModifier) state |= Rml::Input::KM_ALT;
        if (modifiers & Fw::KeyboardMetaModifier) state |= Rml::Input::KM_META;
        return state;
    }

    Rml::Input::KeyIdentifier toRmlKey(const uint8_t key)
    {
        using namespace Rml::Input;
        if (key >= Fw::KeyA && key <= Fw::KeyZ)
            return static_cast<KeyIdentifier>(KI_A + (key - Fw::KeyA));
        if (key >= Fw::Key0 && key <= Fw::Key9)
            return static_cast<KeyIdentifier>(KI_0 + (key - Fw::Key0));
        if (key >= Fw::KeyF1 && key <= Fw::KeyF5)
            return static_cast<KeyIdentifier>(KI_F1 + (key - Fw::KeyF1));
        if (key >= Fw::KeyF6 && key <= Fw::KeyF12)
            return static_cast<KeyIdentifier>(KI_F6 + (key - Fw::KeyF6));
        if (key >= Fw::KeyNumpad0 && key <= Fw::KeyNumpad6)
            return static_cast<KeyIdentifier>(KI_NUMPAD0 + (key - Fw::KeyNumpad0));

        switch (key) {
            case Fw::KeyEscape: return KI_ESCAPE;
            case Fw::KeyTab: return KI_TAB;
            case Fw::KeyBackspace: return KI_BACK;
            case Fw::KeyEnter: return KI_RETURN;
            case Fw::KeyInsert: return KI_INSERT;
            case Fw::KeyDelete: return KI_DELETE;
            case Fw::KeyPause: return KI_PAUSE;
            case Fw::KeyHome: return KI_HOME;
            case Fw::KeyEnd: return KI_END;
            case Fw::KeyPageUp: return KI_PRIOR;
            case Fw::KeyPageDown: return KI_NEXT;
            case Fw::KeyUp: return KI_UP;
            case Fw::KeyDown: return KI_DOWN;
            case Fw::KeyLeft: return KI_LEFT;
            case Fw::KeyRight: return KI_RIGHT;
            case Fw::KeyCtrl: return KI_LCONTROL;
            case Fw::KeyShift: return KI_LSHIFT;
            case Fw::KeyAlt: return KI_LMENU;
            case Fw::KeySpace: return KI_SPACE;
            case Fw::KeyComma: return KI_OEM_COMMA;
            case Fw::KeyMinus: return KI_OEM_MINUS;
            case Fw::KeyPeriod: return KI_OEM_PERIOD;
            case Fw::KeyPlus: return KI_OEM_PLUS;
            case Fw::KeySlash: return KI_OEM_2;
            case Fw::KeySemicolon: return KI_OEM_1;
            case Fw::KeyLeftBracket: return KI_OEM_4;
            case Fw::KeyBackslash: return KI_OEM_5;
            case Fw::KeyRightBracket: return KI_OEM_6;
            case Fw::KeyApostrophe: return KI_OEM_7;
            case Fw::KeyGrave: return KI_OEM_3;
            default: return KI_UNKNOWN;
        }
    }

    // Typing in a text field must not trigger OTClient hotkeys (walking, spells...).
    bool isTextField(Rml::Element* element)
    {
        if (!element)
            return false;
        const auto& tag = element->GetTagName();
        if (tag == "textarea")
            return true;
        if (tag != "input")
            return false;
        const auto type = element->GetAttribute<Rml::String>("type", "text");
        return type == "text" || type == "password" || type == "number" || type == "search";
    }

    int toRmlButton(const Fw::MouseButton button)
    {
        switch (button) {
            case Fw::MouseLeftButton: return 0;
            case Fw::MouseRightButton: return 1;
            case Fw::MouseMidButton: return 2;
            default: return 3;
        }
    }
}

UIRmlWidget::UIRmlWidget()
{
    setFocusable(true);
    g_rml.registerWidget(this);
}

UIRmlWidget::~UIRmlWidget()
{
    releaseContext();
    g_rml.unregisterWidget(this);
}

Size UIRmlWidget::contextSize() const
{
    return { std::max<int>(1, m_rect.width()), std::max<int>(1, m_rect.height()) };
}

bool UIRmlWidget::ensureContext()
{
    if (m_context)
        return true;

    if (!g_rml.isReady())
        return false;

    std::scoped_lock lock(g_rml.mutex());

    m_renderer = std::make_shared<RmlRenderer>();
    m_contextName = fmt::format("otui_{}", g_rml.nextContextId());

    const auto size = contextSize();
    m_context = Rml::CreateContext(m_contextName, { size.width(), size.height() }, m_renderer.get());
    if (!m_context) {
        g_logger.error("[rmlui] unable to create context '{}'", m_contextName);
        m_renderer = nullptr;
        return false;
    }

    // RCSS can offer a light and a full version: @media (theme: efeitos) { ... }
    m_effectsTheme = g_rml.supportsAdvancedEffects();
    m_context->ActivateTheme("efeitos", m_effectsTheme);
    return true;
}

void UIRmlWidget::releaseContext()
{
    if (!m_context)
        return;

    {
        std::scoped_lock lock(g_rml.mutex());

        if (g_debuggerOwner == this) {
            Rml::Debugger::SetVisible(false);
            Rml::Debugger::Shutdown();
            g_debuggerInitialised = false;
            g_debuggerOwner = nullptr;
        }

        m_document = nullptr;
        m_baseStyleSheet = nullptr;
        Rml::RemoveContext(m_contextName);
        m_context = nullptr;
    }

    g_rml.retireRenderer(m_renderer);
    m_renderer = nullptr;
    m_target = nullptr;
}

Rml::Context* UIRmlWidget::getContext()
{
    ensureContext();
    return m_context;
}

bool UIRmlWidget::loadDocument(const std::string& path)
{
    if (!ensureContext())
        return false;

    {
        std::scoped_lock lock(g_rml.mutex());

        if (m_document) {
            m_document->Close();
            m_document = nullptr;
        }

        m_document = m_context->LoadDocument(path);
        if (!m_document) {
            g_logger.error("[rmlui] unable to load '{}'", path);
            m_documentPath.clear();
            return false;
        }

        m_documentPath = path;

        // keep a private copy of the document stylesheets so extra sheets can be swapped
        const auto empty = Rml::Factory::InstanceStyleSheetString("");
        const auto* base = m_document->GetStyleSheetContainer();
        m_baseStyleSheet = base && empty ? base->CombineStyleSheetContainer(*empty) : empty;

        applyStyleSheet();
        m_document->Show();
    }

    callLuaField("onRmlDocumentLoaded", path);
    return true;
}

void UIRmlWidget::unloadDocument()
{
    if (!m_document)
        return;

    std::scoped_lock lock(g_rml.mutex());
    m_document->Close();
    m_document = nullptr;
    m_baseStyleSheet = nullptr;
    m_documentPath.clear();
    if (m_context)
        m_context->Update();
}

bool UIRmlWidget::reloadDocument()
{
    if (m_documentPath.empty())
        return false;

    const std::string path = m_documentPath;
    {
        std::scoped_lock lock(g_rml.mutex());
        Rml::Factory::ClearStyleSheetCache();
        Rml::Factory::ClearTemplateCache();
    }
    return loadDocument(path);
}

void UIRmlWidget::setDebuggerVisible(const bool visible)
{
    m_debuggerVisible = visible;
    if (!ensureContext())
        return;

    std::scoped_lock lock(g_rml.mutex());

    if (visible) {
        if (!g_debuggerInitialised) {
            g_debuggerInitialised = Rml::Debugger::Initialise(m_context);
        } else {
            Rml::Debugger::SetContext(m_context);
        }
        g_debuggerOwner = this;
        Rml::Debugger::SetVisible(true);
    } else if (g_debuggerOwner == this) {
        Rml::Debugger::SetVisible(false);
    }
}

bool UIRmlWidget::setExtraStyleSheet(const std::string& rcss)
{
    m_extraStyleSheet = rcss;
    std::scoped_lock lock(g_rml.mutex());
    applyStyleSheet();
    return true;
}

void UIRmlWidget::applyStyleSheet()
{
    if (!m_document || !m_baseStyleSheet)
        return;

    if (m_extraStyleSheet.empty()) {
        m_document->SetStyleSheetContainer(m_baseStyleSheet->CombineStyleSheetContainer(*Rml::Factory::InstanceStyleSheetString("")));
        return;
    }

    const auto extra = Rml::Factory::InstanceStyleSheetString(m_extraStyleSheet);
    if (!extra) {
        g_logger.warning("[rmlui] extra stylesheet rejected for '{}'", m_documentPath);
        return;
    }
    m_document->SetStyleSheetContainer(m_baseStyleSheet->CombineStyleSheetContainer(*extra));
}

void UIRmlWidget::tick()
{
    if (!m_context || !isVisible())
        return;

    callLuaField("onRmlUpdate");

    if (!m_context) // the callback may have destroyed us
        return;

    {
        std::scoped_lock lock(g_rml.mutex());

        const auto size = contextSize();
        const Rml::Vector2i dimensions{ size.width(), size.height() };
        if (m_context->GetDimensions() != dimensions)
            m_context->SetDimensions(dimensions);

        const bool effects = g_rml.supportsAdvancedEffects();
        if (effects != m_effectsTheme) {
            m_effectsTheme = effects;
            m_context->ActivateTheme("efeitos", effects);
        }

        m_context->Update();
    }

    if (m_context)
        callLuaField("onRmlLayout");
}

void UIRmlWidget::drawSelf(const DrawPoolType drawPane)
{
    UIWidget::drawSelf(drawPane);

    if (!m_context || !m_renderer || m_rect.width() <= 0 || m_rect.height() <= 0)
        return;

    const auto size = contextSize();
    if (!m_target || m_target->getSize() != size)
        m_target = std::make_shared<RmlTargetTexture>(size);

    {
        std::scoped_lock lock(g_rml.mutex());
        m_renderer->beginFrame(m_target);
        m_context->Render();
        m_renderer->endFrame();
    }

    // Replays the recorded frame into m_target on the graphics thread, then the draw pool
    // draws the texture like any other widget image (clipping, order and opacity included).
    g_drawPool.addAction([renderer = m_renderer] { renderer->replay(); });
    g_drawPool.setCompositionMode(CompositionMode::PREMULTIPLIED, true);
    g_drawPool.addTexturedRect(m_rect, m_target, Rect(Point(0, 0), size), Color::white);
}

void UIRmlWidget::parseCustomStyle(const OTMLNodePtr& styleNode)
{
    UIWidget::parseCustomStyle(styleNode);

    for (const auto& node : styleNode->children()) {
        if (node->tag() == "rml-source")
            loadDocument(node->value());
        else if (node->tag() == "rml-debugger")
            setDebuggerVisible(node->value<bool>());
    }
}

std::string UIRmlWidget::findDragMode() const
{
    for (auto* element = m_context ? m_context->GetHoverElement() : nullptr; element; element = element->GetParentNode()) {
        if (const auto* value = element->GetAttribute("data-drag"))
            return value->Get<Rml::String>();
    }
    return {};
}

void UIRmlWidget::onHoverChange(const bool hovered)
{
    UIWidget::onHoverChange(hovered);
    if (!hovered && m_context && !m_dragging) {
        std::scoped_lock lock(g_rml.mutex());
        m_context->ProcessMouseLeave();
    }
}

void UIRmlWidget::onFocusChange(const bool focused, const Fw::FocusReason reason)
{
    UIWidget::onFocusChange(focused, reason);
    if (!focused && m_context) {
        std::scoped_lock lock(g_rml.mutex());
        if (auto* focus = m_context->GetFocusElement())
            focus->Blur();
    }
}

bool UIRmlWidget::onMousePress(const Point& mousePos, const Fw::MouseButton button)
{
    bool consumed = false;
    if (m_context) {
        std::scoped_lock lock(g_rml.mutex());
        m_context->ProcessMouseMove(localX(mousePos), localY(mousePos), 0);
        consumed = !m_context->ProcessMouseButtonDown(toRmlButton(button), 0);

        if (button == Fw::MouseLeftButton && findDragMode() == "window") {
            // anchors (e.g. RmlWindow's centerIn) would put the widget back on the next layout
            breakAnchors();
            m_dragging = true;
            m_dragOffset = mousePos - getPosition();
            consumed = true;
        }
    }

    // a document fills the widget: presses on it belong to it (OTUI focus, raise, etc.)
    return UIWidget::onMousePress(mousePos, button) || consumed || hasDocument();
}

bool UIRmlWidget::onMouseRelease(const Point& mousePos, const Fw::MouseButton button)
{
    bool consumed = false;
    if (m_context) {
        std::scoped_lock lock(g_rml.mutex());
        consumed = !m_context->ProcessMouseButtonUp(toRmlButton(button), 0);
    }

    if (button == Fw::MouseLeftButton && m_dragging) {
        m_dragging = false;
        consumed = true;
    }

    return UIWidget::onMouseRelease(mousePos, button) || consumed;
}

bool UIRmlWidget::onMouseMove(const Point& mousePos, const Point& mouseMoved)
{
    if (m_dragging) {
        setPosition(mousePos - m_dragOffset);
        bindRectToParent();
        return true;
    }

    if (m_context) {
        std::scoped_lock lock(g_rml.mutex());
        m_context->ProcessMouseMove(localX(mousePos), localY(mousePos), 0);
    }

    return UIWidget::onMouseMove(mousePos, mouseMoved);
}

bool UIRmlWidget::onMouseWheel(const Point& mousePos, const Fw::MouseWheelDirection direction)
{
    if (m_context && direction != Fw::MouseNoWheel) {
        std::scoped_lock lock(g_rml.mutex());
        m_context->ProcessMouseMove(localX(mousePos), localY(mousePos), 0);
        if (!m_context->ProcessMouseWheel(direction == Fw::MouseWheelUp ? -1.f : 1.f, 0))
            return true;
    }
    return UIWidget::onMouseWheel(mousePos, direction);
}

bool UIRmlWidget::onKeyText(const std::string_view keyText)
{
    if (m_context && !keyText.empty() && static_cast<uint8_t>(keyText.front()) >= 32 && static_cast<uint8_t>(keyText.front()) != Fw::KeyDel) {
        std::scoped_lock lock(g_rml.mutex());
        if (auto* focus = m_context->GetFocusElement()) {
            const bool notConsumed = m_context->ProcessTextInput(RmlManager::utf8(keyText));
            if (!notConsumed || isTextField(focus))
                return true;
        }
    }
    return UIWidget::onKeyText(keyText);
}

bool UIRmlWidget::onKeyPress(const uint8_t keyCode, const int keyboardModifiers, const int autoRepeatTicks)
{
    if (m_context) {
        std::scoped_lock lock(g_rml.mutex());
        const auto key = toRmlKey(keyCode);
        if (auto* focus = m_context->GetFocusElement(); focus && key != Rml::Input::KI_UNKNOWN) {
            const bool notConsumed = m_context->ProcessKeyDown(key, toRmlModifiers(keyboardModifiers));
            if (!notConsumed || (isTextField(focus) && key != Rml::Input::KI_ESCAPE))
                return true;
        }
    }
    return UIWidget::onKeyPress(keyCode, keyboardModifiers, autoRepeatTicks);
}

bool UIRmlWidget::onKeyUp(const uint8_t keyCode, const int keyboardModifiers)
{
    if (m_context) {
        std::scoped_lock lock(g_rml.mutex());
        const auto key = toRmlKey(keyCode);
        if (key != Rml::Input::KI_UNKNOWN)
            m_context->ProcessKeyUp(key, toRmlModifiers(keyboardModifiers));
    }
    return UIWidget::onKeyUp(keyCode, keyboardModifiers);
}
