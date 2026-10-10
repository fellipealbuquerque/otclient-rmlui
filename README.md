# OTClient with RmlUi

**English** | [Português (Brasil)](README.pt-BR.md)

OTClient with **[RmlUi 6.3](https://github.com/mikke89/RmlUi)** built in: screens written in
**HTML (.rml) and CSS (.rcss)**, with Lua data binding, living side by side with the usual OTUI windows.
The API follows the Ninja World Online client (`g_rml`, `UIRmlWidget`, `RmlWindow`).

> [!WARNING]
> **No support is provided.** This project is shared as is: issues, questions and
> requests for help will not be answered. Use it, fork it and adapt it on your own.

[![Showcase](docs/images/rmlui/showcase.gif)](docs/images/rmlui/showcase.mp4)

[Full video (MP4)](docs/images/rmlui/showcase.mp4)

## What was added

- `UIRmlWidget` / `RmlWindow` / `RmlWidget`: an RmlUi document inside the OTUI tree
  (`rml-source:` in .otui or `widget:loadDocument()` in Lua), with mouse, keyboard, focus,
  title-bar dragging (`data-drag="window"`) and the RmlUi visual debugger.
- `g_rml`: initialization, fonts (`data/fonts/rml`), Latin-1/UTF-8 conversion, widget list.
- Game images straight in the HTML: `item/<id>.png`, `effect/<id>.png`,
  `outfit/<looktype>-<colors>-<addons>-<direction>.png`, with crisp upscaling (`@2x`..`@8x`).
- Full RmlUi 6 effects on OpenGL 3.3 (gradients, shadows, filters, masks, animations and
  transitions) and the `efeitos` theme for light/full versions of the CSS.
- Native OTUI widgets inside RML (minimap, item, creature) with `Rml.host`.
- Documented Lua helpers in [`modules/corelib/rml.lua`](modules/corelib/rml.lua), including an
  OTUI → RmlUi mapping.
- `.dat/.spr` exported by Object Builder: the `Tibia.otfi` next to them is honored.

## Examples

| Module | Shortcut | Content |
|---|---|---|
| `client_rmlshowcase` | **RmlUi** button on the top menu / **Ctrl+Shift+U** | tabs: data binding, forms, sprites, OTUI in RML (minimap), images and carousel, effects, OTUI to RmlUi, Lua and DOM |
| `client_rmloptions` | **Ctrl+Shift+O** | client options in RmlUi |
| `game_rmloutfit` | **Ctrl+Shift+F** | outfit with preview, colors, addons and looktypes |
| `game_rmlminimap` | **Ctrl+Shift+M** | minimap with RML controls |
| `client_rmldemo` | **Ctrl+Shift+R** | minimal example |

To see sprites and the minimap outside the game, put the game files in
`data/things/<version>/` (`Tibia.dat`, `Tibia.spr`, `Tibia.otfi` and, if you have one,
`minimap.otmm`) and use the showcase **Carregar assets** (load assets) button.

## Quick start

```lua
local window, model = Rml.createWindow{
    document = '/modules/my_module/window.rml',
    model = 'mine',
    data = {
        count = 0,
        Add = function(event, delta) model.count = model.count + delta end,
    },
}
```

```html
<rml>
<head><link type="text/rcss" href="/styles/rml/otc.rcss"/></head>
<body>
  <div class="otc-window" data-model="mine">
    <header class="otc-header" data-drag="window"><span class="otc-title">My window</span></header>
    <p>Clicks: {{count}}</p>
    <button class="btn primary" data-event-click="Add(1)">+1</button>
    <img src="item/2160.png"/>
  </div>
</body>
</rml>
```

Full documentation (API, RML features, architecture, in Portuguese): [docs/RMLUI.md](docs/RMLUI.md).

## Building

Windows with Visual Studio 18 (MSVC v145) and [vcpkg](https://github.com/microsoft/vcpkg)
(`VCPKG_ROOT`, default `C:\vcpkg`):

```bat
build-rml.bat          :: configure and build
build-rml.bat build    :: build only
```

CMake downloads RmlUi 6.3 and builds it with the Lua plugin on top of the client's LuaJIT.
`-DTOGGLE_RMLUI=OFF` disables the integration.

## Credits

Based on **OTClient - Redemption** by mehah: <https://github.com/mehah/otclient>
(derived from the original [OTClient](https://github.com/edubart/otclient) by edubart).
The base project's README, documentation and roadmap live in its repository.

- [RmlUi](https://github.com/mikke89/RmlUi) — MIT ([src/framework/rml/RMLUI_LICENSE.txt](src/framework/rml/RMLUI_LICENSE.txt))
- [Barlow](https://github.com/jpt/barlow) and [Share Tech Mono](https://fonts.google.com/specimen/Share+Tech+Mono) fonts — SIL Open Font License (`data/fonts/rml`)

MIT License, see [LICENSE](LICENSE).
