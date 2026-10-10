-- RmlUi demo: a free RmlWindow driven by a Lua data model.
-- Open with Ctrl+Shift+R or the top menu button.

local window, model, button

local function thingsLoaded()
    return g_things and g_things.isDatLoaded and g_things.isDatLoaded() or false
end

local function close()
    if window then
        window:destroy()
        window = nil
        model = nil
    end
end

local function open()
    if window then
        window:raise()
        window:focus()
        return
    end

    if not g_rml or not g_rml.isReady() then
        g_logger.warning('RmlUi is not available in this client build')
        return
    end

    window = g_ui.createWidget('RmlWindow', rootWidget)
    window:setId('rmlDemoWindow')
    window:setSize({ width = 560, height = 360 })
    window.onEscape = close

    -- The model must exist before the document that references it (data-model="demo").
    model = window:getContext():OpenDataModel('demo', {
        title = g_rml.utf8('RmlUi no OTClient'),
        count = 0,
        progress = 0,
        name = '',
        selected = 0,
        thingsLoaded = thingsLoaded(),
        backend = g_rml.getBackendName(),
        items = {
            { id = 3031, name = 'gold coin' }, { id = 3035, name = 'platinum coin' },
            { id = 3043, name = 'crystal coin' }, { id = 2148, name = 'gold coin (8.6)' },
            { id = 2152, name = 'platinum coin (8.6)' }, { id = 2160, name = 'crystal coin (8.6)' },
        },
        Add = function(event, delta)
            model.count = model.count + delta
            model.progress = math.max(0, math.min(10, model.count))
        end,
        Select = function(event, id) model.selected = id end,
        Close = function() addEvent(close) end,
        ToggleDebugger = function() window:setDebuggerVisible(not window:isDebuggerVisible()) end,
        Reload = function() addEvent(function() if window then window:reloadDocument() end end) end,
    })

    window.onRmlUpdate = function()
        if model then
            local loaded = thingsLoaded()
            if model.thingsLoaded ~= loaded then model.thingsLoaded = loaded end
            local backend = g_rml.getBackendName()
            if model.backend ~= backend then model.backend = backend end
        end
    end

    if not window:loadDocument('/modules/client_rmldemo/rmldemo.rml') then
        close()
        return
    end
    window:focus()
end

function toggle()
    if window then close() else open() end
end

function init()
    g_keyboard.bindKeyDown('Ctrl+Shift+R', toggle)
    if modules.client_topmenu then
        button = modules.client_topmenu.addLeftButton('rmlDemoButton', tr('RmlUi demo') .. ' (Ctrl+Shift+R)',
            '/images/topbuttons/debug', toggle)
    end
end

function terminate()
    g_keyboard.unbindKeyDown('Ctrl+Shift+R')
    close()
    if button then
        button:destroy()
        button = nil
    end
end
