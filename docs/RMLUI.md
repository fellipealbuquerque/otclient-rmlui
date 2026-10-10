# RmlUi no OTClient (HTML/CSS dentro do OTUI)

Este fork do [mehah/otclient](https://github.com/mehah/otclient) embute o **RmlUi 6.3** com a mesma
API usada pelo cliente do Ninja World Online (`g_rml`, `UIRmlWidget`, `RmlWindow`, `data-drag`,
imagens `item/`/`outfit/`, tema `efeitos`). Um documento `.rml` + `.rcss` é desenhado dentro de
um widget OTUI e convive com o resto da interface (ordem, recorte, foco, teclado e mouse).

## Exemplos

| Módulo | Abrir | O que mostra |
|---|---|---|
| `client_rmlshowcase` | botão **RmlUi** no menu superior ou **Ctrl+Shift+U** | janela com abas: data binding, formulários, sprites, widgets OTUI dentro do RML (minimapa, item, criatura), imagens (img, tint, decorator, nine-patch, spritesheet, carrossel), efeitos de CSS, comparação OTUI para RmlUi, Lua e DOM |
| `client_rmloptions` | **Ctrl+Shift+O** ou pelo showcase | as opções do cliente refeitas em RmlUi (mesmas chaves de `client_options`) |
| `game_rmloutfit` | **Ctrl+Shift+F** ou pelo showcase | outfit: preview girável, cores por parte, addons, looktypes; no jogo envia `changeOutfit` |
| `game_rmlminimap` | **Ctrl+Shift+M** ou pelo showcase | minimapa: controles em RML (zoom, andares, ir para coordenada) com o `UIMinimap` nativo hospedado; fora do jogo carrega `data/things/<versão>/minimap.otmm` ou o `/minimap.otmm` salvo |
| `client_rmldemo` | **Ctrl+Shift+R** | exemplo mínimo |

Helpers Lua: **`modules/corelib/rml.lua`** (global `Rml`), escrito como documentação: tabela
OTUI × RmlUi, data model, texto, imagens, DOM, `Rml.host` e `Rml.watch`. Os exemplos usam só ele.

### Capturas

Coloque os prints em `docs/images/rmlui/` com estes nomes:

| Arquivo | Conteúdo |
|---|---|
| `showcase-inicio.png` | showcase, aba Início |
| `showcase-binding.png` | aba Data binding |
| `showcase-formularios.png` | aba Formulários |
| `showcase-sprites.png` | aba Sprites (com assets carregados) |
| `showcase-otui-no-rml.png` | aba OTUI no RML (minimapa no jogo) |
| `showcase-imagens.png` | aba Imagens (carrossel) |
| `showcase-efeitos.png` | aba Efeitos |
| `showcase-otui-para-rmlui.png` | aba OTUI para RmlUi |
| `opcoes.png` | janela de opções |
| `outfit.png` | janela de outfit |
| `minimapa.png` | janela de minimapa |

![Showcase](images/rmlui/showcase-inicio.png)
![Sprites](images/rmlui/showcase-sprites.png)
![Opções](images/rmlui/opcoes.png)
![Outfit](images/rmlui/outfit.png)
![Minimapa](images/rmlui/minimapa.png)

## Build

Windows, Visual Studio 18 (MSVC v145) + vcpkg (`VCPKG_ROOT`, padrão `C:\vcpkg`):

```bat
build-rml.bat          :: configura e compila (preset rml-release de CMakeUserPresets.json)
build-rml.bat build    :: só compila
```

O CMake baixa o RmlUi 6.3 (`FetchContent`) e compila com o plugin Lua sobre o **LuaJIT** do
próprio cliente e com o FreeType do vcpkg. Opção: `-DTOGGLE_RMLUI=OFF` desliga tudo (Android e
WebAssembly ficam sempre sem RmlUi). Fontes: `data/fonts/rml/*.ttf` (Barlow, licença OFL).

## Uso rápido

```lua
local window = g_ui.createWidget('RmlWindow', rootWidget)
window:setSize({ width = 560, height = 360 })
window.onEscape = function() window:destroy() end

-- o modelo precisa existir antes do documento que o usa (data-model="demo")
local model = window:getContext():OpenDataModel('demo', {
  count = 0,
  Add = function(event, delta) model.count = model.count + delta end,
})
window:loadDocument('/modules/meu_modulo/janela.rml')
```

```html
<rml>
<head><link type="text/rcss" href="janela.rcss"/></head>
<body>
  <div data-model="demo">
    <header data-drag="window">Título <button data-drag="none" data-event-click="Close()">×</button></header>
    <p>Cliques: {{count}}</p>
    <button data-event-click="Add(1)">+1</button>
    <img src="item/3031.png"/>   <!-- sprite do jogo -->
  </div>
</body>
</rml>
```

Também dá para declarar no OTUI:

```otui
RmlWidget
  id: painel
  size: 300 200
  rml-source: /modules/meu_modulo/painel.rml
  rml-debugger: false
```

## API

### `g_rml`

| Função | Descrição |
|---|---|
| `isReady()` | inicializa o RmlUi na primeira chamada; `false` se não houver backend |
| `supportsAdvancedEffects()` | `true` com o backend OpenGL 3.3 (filtros, sombras, gradientes, máscaras) |
| `getBackendName()` | nome do backend |
| `setReportEnabled(bool)` / `isReportEnabled()` | sinalizador (o relatório de desempenho do NWO não foi portado) |
| `loadFontFace(caminho)` | carrega uma fonte TTF/OTF extra |
| `utf8(texto)` / `latin1(texto)` | Latin-1 (servidor/OTUI) ⇄ UTF-8 (RmlUi) |
| `getWidgetCount()` / `getWidgets()` | widgets RmlUi existentes |

### `UIRmlWidget` (base de `RmlWidget` e `RmlWindow`)

| Método | Descrição |
|---|---|
| `loadDocument(caminho)` → bool | carrega e mostra o documento (dispara `onRmlDocumentLoaded(caminho)`) |
| `unloadDocument()` / `reloadDocument()` | descarrega / recarrega limpando o cache de estilos |
| `hasDocument()` / `getDocumentPath()` | |
| `setDebuggerVisible(bool)` / `isDebuggerVisible()` | depurador visual do RmlUi dentro do widget |
| `setExtraStyleSheet(rcss)` | RCSS extra aplicado por cima do documento (temas); `''` remove |
| `getDocument()` | `Document` do plugin Lua do RmlUi (`GetElementById`, `QuerySelector`, ...) |
| `getContext()` | `Context` do plugin Lua (`OpenDataModel`, `ProcessMouseMove`, ...) |

Eventos Lua chamados no widget: `onRmlDocumentLoaded(path)`, `onRmlUpdate()` (antes do
`Context:Update()`), `onRmlLayout()` (depois do layout, para medir `offset_*`/`scroll_*`).
`RmlWindow` chama `self.onEscape` com Esc.

A API DOM completa é a do [plugin Lua oficial do RmlUi](https://mikke89.github.io/RmlUiDoc/pages/lua_manual.html)
(`rmlui`, `Context`, `Document`, `Element`, `Event`, controles de formulário, data models).

### Recursos no RML/RCSS

- `data-drag="window"`: o elemento vira alça que arrasta o widget OTUI; `data-drag="none"` cancela
  num descendente (botões dentro da barra de título).
- Imagens geradas das sprites do jogo (depois de `.dat/.spr` carregados):
  `item/<id>.png`, `effect/<id>.png`,
  `outfit/<looktype>-<head>-<body>-<legs>-<feet>-<addons>-<direção>.png` (direção 0=N, 1=L, 2=S, 3=O;
  addons somam 1 e 2; cores pela paleta de outfits). Funciona com `.dat/.spr` clássico e com
  assets protobuf (12.x+). Pedida antes de os assets carregarem, a imagem fica transparente e é
  recarregada sozinha quando eles ficam prontos; sprites ocupadas são tentadas de novo. Dê
  `width`/`height` ao `<img>` no RCSS para o tamanho não depender da imagem carregada.
  Sufixo `@2x`..`@8x` amplia sem borrar: `item/2160@3x.png`.
- Widgets OTUI nativos dentro do documento: reserve o espaço com um elemento e chame
  `Rml.host(janela, 'idDoElemento', g_ui.createWidget('Minimap'))`.
- `.dat/.spr` salvos pelo Object Builder: deixe o `Tibia.otfi` ao lado; o `game_things` lê as
  opções dele (transparency, extended, frame groups).
- Tema `efeitos`, ativo quando o backend completo existe:
  `@media (theme: efeitos) { .x { box-shadow: ...; decorator: linear-gradient(...); filter: blur(2dp); } }`.
- Campos de texto (`<input type="text">`, `<textarea>`) seguram o teclado: digitar não dispara
  atalhos do jogo.

## Como funciona

```
thread de UI/Lua (poll)              thread de desenho da UI           thread GL (principal)
--------------------------           --------------------------        ---------------------
tick 16 ms: onRmlUpdate              UIRmlWidget::drawSelf             DrawPool::draw
  Context::SetDimensions               Context::Render()                 ação: RmlRenderer::replay()
  Context::Update                       -> RmlRenderer grava comandos       RenderInterface_GL3 executa
  onRmlLayout                             com handles "proxy"              e compõe numa textura do widget
eventos de mouse/teclado ->           g_drawPool.addAction(replay)        retângulo texturizado (alfa
  Context::Process*                   g_drawPool.addTexturedRect(...)     pré-multiplicado) no lugar do widget
```

- `src/framework/rml/rmlmanager.*`: `g_rml`, interfaces de sistema/arquivo (lê pelo
  `g_resources`), fontes, imagens virtuais, tick.
- `src/framework/rml/uirmlwidget.*`: o widget (um `Rml::Context` por widget, entrada, arrasto,
  folha extra, debugger).
- `src/framework/rml/rmlrenderer.*`: grava as chamadas do RmlUi e reproduz na thread GL,
  mapeando handles; salva/restaura o estado GL que o painter guarda em cache.
- `src/framework/rml/RmlUi_Renderer_GL3.*`: backend oficial do RmlUi 6.3 com três mudanças
  (composição num framebuffer alvo, shaders compartilhados entre instâncias, limpeza de estado).
- `src/client/rmlimages.*`: monta as imagens `item/`, `effect/`, `outfit/` a partir das sprites.
- `CompositionMode::PREMULTIPLIED` no painter: o RmlUi 6 trabalha com alfa pré-multiplicado.

O RmlUi não é thread-safe: todas as chamadas do C++ passam por `g_rml.mutex()`. As chamadas
feitas pelo Lua (data models, DOM) acontecem na thread de Lua, que nunca roda junto com o desenho
da UI (o laço do cliente espera o desenho terminar antes do próximo `poll`).

## Diferenças em relação ao NWO

Implementado com a mesma API e comportamento. Ainda não portado:
- fallback OpenGL 2 (`NWO_RML_GL2`): sem GL 3.3 os widgets ficam vazios e `g_rml.isReady()` passa
  a `false` depois do primeiro quadro;
- fontes bitmap `.otfont` do OTClient dentro do RmlUi (aqui só TTF/OTF);
- relatório `rmluireport.log` e variáveis `NWO_RML_PROFILE`/`NWO_RML_TRACE`;
- as bibliotecas Lua do próprio jogo do NWO (`RmlAppearance`, `RmlMessageBox`, ...).
