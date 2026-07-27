# {{project-name}}

{{project_description}}

## Documentation

This project documentation is rooted here.

- [Submodule update guide](docs/cpp-core-submodule-update.md)

## Quick Start

Build and run the standard targets:

- make
- make release
- make lto
- make instrument
- make bear
- make verify-submodule-pin

If using ui_type=sdl3, ensure SDL3 development packages are installed.

## Dependency Setup Notes

- cpp-core is managed as a git submodule at third_party/cpp-core.
- ImGui setup is handled by ci/init_imgui.sh when generated with ui_type=imgui.
- Vulkan-Headers setup is handled by ci/init_vulkan_headers.sh only when generated with ui_type=imgui and imgui_backend=glfw_vulkan.
- SDL3 dependency checks are handled by ci/init_sdl3.sh when generated with ui_type=sdl3.