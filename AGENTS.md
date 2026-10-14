# AGENTS.md

Legacy DirectX 11 class demo (Spanish, UNLAM-style). No README, tests, lint, CI, or package manager. Single VS solution, header-heavy C++.

## Active code vs. legacy copies

- Active build: `DXClase2.sln` → `DXClase2/DXClase2.vcxproj` (Toolset `v143`, `Debug|Win32`). Edit here.
- `Objetos/plantilla/` is a stale template copy (`v120` toolset, own `.sln`, `MObject.h`/`MCube.h` family). Do not port or sync changes unless asked.
- Orphan headers at `DXClase2/MeshCube.h`, `MeshSphere.h`, `MeshTorus.h`, `MeshCone.h`, `MeshCilinder.h` are NOT in the `.vcxproj` and not `#include`d. Ignore them.

## Architecture (all logic in headers)

- Entry: `DXClase2/main.cpp` (`WinMain` → `InitDevice` → `InitScene` → `InitDirectInput` → `UpdateScene`/`DetectInput`/`DrawScene` loop, `ESC` quits).
- `DXClase2/DXRender.h` owns device/swapchain, input, camera, and all `MPrimitives*` scene objects.
- `DXClase2/MeshPrimitives.h` is the only mesh system: overloaded `MPrimitives` ctors build cube/sphere/torus/cylinder(`createCilinder`, sic)/terrain-from-BMP/OBJ-txt/billboard/GUI-bitmap. `Camara.h` (sic), `Light.h`, `Constants.h` (shader enum) complete it.

## Build

- Requires Windows + Visual Studio with `v143` + legacy **DirectX SDK June 2010** (`d3d11/d3dx11/d3dx10/dinput8`). Only `Debug|Win32` sets the SDK `IncludePath`/`LibraryPath` (`C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\...`) and links `d3d11.lib;d3dx11d.lib;D3DCompiler.lib;dxgi.lib;dxguid.lib`. `Release|Win32` has no SDK paths/link libs and will fail.
- `Win32`/`x86` only, `SubSystem: Windows`. No CMake/CL alternative verified.
- Example: `msbuild DXClase2.sln /p:Configuration=Debug /p:Platform=Win32`
- Commit `*.sdf/*.suo/*.user`, `Debug/`, `Objetos/**/Debug/`, `*.exe/*.pdb/*.ilk` build leftovers exist in-tree; do not rely on or clean up unasked.

## Runtime gotcha: working directory + loose assets

- Shaders/textures/heightmaps load by **relative path at runtime** via `D3DX11CompileFromFile` / `D3DX11CreateShaderResourceViewFromFile` / `fopen`, with return values largely unchecked. Run with CWD = `DXClase2/` (or copy the exe next to `Texturas/`, `Models/`, `*.vs`, `*.ps`). Missing files = black/missing geometry, not an error dialog.
- Shader set: `vertexShader.vs` + `pixelShader*.ps` selected by `Constants.h` enum (`ShaderDiffuse/Specular/Normalmap/GUI/Multitextura`). `MeshPrimitives.h:278` also hardcodes `Texturas/rock.jpg` as second multitexture layer.
- Terrain heightmap must be 24-bit BMP (parsed via `BITMAPFILEHEADER`); active one is `Texturas/heightmap01.bmp` (`TEXTURE_REPEAT=8`, height `/= 15.0f`). `Models/*.txt` (`BarrilTriangle.txt`, `bg.txt`) use the custom `VertexCount:` text format in `LoadModel`, not raw `.obj` (`Small_Building_1.obj` at project root is unused).

# Behavioral Guidelines

Behavioral guidelines to reduce common LLM coding mistakes. Merge with project-specific instructions as needed.

**Tradeoff:** These guidelines bias toward caution over speed. For trivial tasks, use judgment.

## 1. Think Before Coding

**Don't assume. Don't hide confusion. Surface tradeoffs.**

Before implementing:
- State your assumptions explicitly. If uncertain, ask.
- If multiple interpretations exist, present them - don't pick silently.
- If a simpler approach exists, say so. Push back when warranted.
- If something is unclear, stop. Name what's confusing. Ask.

## 2. Simplicity First

**Minimum code that solves the problem. Nothing speculative.**

- No features beyond what was asked.
- No abstractions for single-use code.
- No "flexibility" or "configurability" that wasn't requested.
- No error handling for impossible scenarios.
- If you write 200 lines and it could be 50, rewrite it.

Ask yourself: "Would a senior engineer say this is overcomplicated?" If yes, simplify.

## 3. Surgical Changes

**Touch only what you must. Clean up only your own mess.**

When editing existing code:
- Don't "improve" adjacent code, comments, or formatting.
- Don't refactor things that aren't broken.
- Match existing style, even if you'd do it differently.
- If you notice unrelated dead code, mention it - don't delete it.

When your changes create orphans:
- Remove imports/variables/functions that YOUR changes made unused.
- Don't remove pre-existing dead code unless asked.

The test: Every changed line should trace directly to the user's request.

## 4. Goal-Driven Execution

**Define success criteria. Loop until verified.**

Transform tasks into verifiable goals:
- "Add validation" → "Write tests for invalid inputs, then make them pass"
- "Fix the bug" → "Write a test that reproduces it, then make it pass"
- "Refactor X" → "Ensure tests pass before and after"

For multi-step tasks, state a brief plan:
```
1. [Step] → verify: [check]
2. [Step] → verify: [check]
3. [Step] → verify: [check]
```

Strong success criteria let you loop independently. Weak criteria ("make it work") require constant clarification.

---

**These guidelines are working if:** fewer unnecessary changes in diffs, fewer rewrites due to overcomplication, and clarifying questions come before implementation rather than after mistakes.