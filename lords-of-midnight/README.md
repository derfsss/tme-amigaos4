# lords-of-midnight — The Lords of Midnight for AmigaOS4

**Source:** https://github.com/ChilliHugger/The-Lords-Of-Midnight
**Engine:** axmol 2.3.1 (from ../axmol/)
**Runtime:** clib4 (`-mcrt=clib4`)

## What

Build The Lords of Midnight against the AmigaOS4-ported axmol engine.
This is the final integration target.

## Work required

1. Build against ported axmol with Vulkan backend
2. Verify 3 custom shaders (time-of-day effects) through VulkanOS4's SPIR-V pipeline
3. Fix game-specific issues:
   - Path handling (PROGDIR: for assets)
   - Input mapping (keyboard/mouse via GLFW OS4)
   - Save game paths
4. Add AmigaOS4 integration:
   - Tooltypes (FULLSCREEN, WIDTH, HEIGHT, RENDERER=GPU/SOFTWARE)
   - PrefsObjects for persistent settings
   - Icon file (.info with truecolor PNG)
   - Version string ($VER:)
5. Create distribution (.lha archive)

## Game features used

- 2D sprite rendering (primary)
- Custom shaders (1 vertex, 2 fragment for time-of-day)
- TMX tiled maps
- UI widgets (ScrollView, Button, Label)
- Keyboard/mouse input
- Basic audio
- File I/O (save games)
- NO physics, 3D, networking, or scripting
