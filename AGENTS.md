# Repository Guidelines

## Project Structure & Module Organization
- `src/`: Main C++ sources (app, core, ecs, render, systems, event, input, ui, world). Entry point: `src/main.cpp`.
- `data/`: Runtime assets copied next to the binary (`build/bin/data`).
- `lib/`: Third‑party headers vendored locally (e.g., `FastNoiseLite.h`).
- `build/`: CMake build artifacts and the game binary in `build/bin/main`.
- Docs: `README.md`, `DESIGN_ECS.md`, and `zdocs/`.

## Build, Test, and Development Commands
- Configure: `cmake -S . -B build` (first configure fetches SFML/ImGui/EnTT via FetchContent).
- Build: `cmake --build build -j`.
- Run: `./build/bin/main`.
- Clean: `cmake --build build --target clean` (or remove `build/`).

## Coding Style & Naming Conventions
- Language: C++17 (`target_compile_features` enforces it).
- Formatting: `clang-format` with repo `.clang-format` (LLVM base, 4‑space indent, 100‑col limit, attached braces, right‑aligned pointers). Run: `clang-format -i src/**/*.cpp src/**/*.h`.
- Naming: Types `PascalCase`; functions/variables `camelCase`; private members prefixed with `_` (see `Renderer.h`).
- Includes: Prefer project includes without `../`; keep includes sorted (see `.clang-format` `SortIncludes`).

## Testing Guidelines
- Current status: No formal unit tests in repo.
- Preferred: Catch2 or doctest + CTest. Place tests under `tests/`, name `*_tests.cpp`, integrate with `add_subdirectory(tests)` and `enable_testing()`.
- Runtime checks: Validate app boots and renders a window; inspect logs in `build/bin/logs/`.

## Commit & Pull Request Guidelines
- Commits: Follow Conventional Commits: `type(scope): summary`.
  - Examples: `feat(core): add updatable interface`, `perf(core): add performance timers`, `refactor(graphics): improve API`.
- PRs: Keep scope focused; describe purpose, approach, and testing steps. Link related issues. Include screenshots/gifs for UI changes. Pass build and formatting checks.

## Security & Configuration Tips
- First configure requires network to fetch dependencies; subsequent builds use the local cache in `build/_deps/`.
- Do not commit build artifacts or secrets. Asset files belong under `data/` and are copied post‑build via CMake.
