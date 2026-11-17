# R.E.P.O. Runtime Research

A C++ reverse-engineering study of the Unity/Mono runtime used by R.E.P.O.

## What I explored

- Resolving Mono assemblies, classes, fields and methods from the running process.
- Walking managed object lists to inspect players, enemies, items and valuables.
- Projecting world positions into a Dear ImGui overlay.
- Calling managed methods and observing how game state is represented at runtime.

The current prototype includes in-game overlays and writes to managed game state. It is experimental, version-dependent and intended as a learning project rather than a supported mod.

## Project layout

- `mono/` contains the Mono API headers used by the project.
- `scanners.*` provides helpers for locating runtime objects and methods.
- `overlay.*` and `main.cpp` contain the overlay and runtime-inspection prototype.

The repository includes a Visual Studio solution and vcpkg manifests for Windows development. The game and its runtime are not included.
