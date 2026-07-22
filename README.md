# C++26 Core Project Template

A standalone cargo-generate template repository for bootstrapping C++26 module projects that use the same makefile techniques as aoc-cpp and pull cpp-core as a submodule dependency.

## What this template generates

Generated projects include:
- C++26 module-first structure (`src/*.cppm` + `src/main.cpp`)
- Dynamic module discovery for both project modules and cpp-core modules
- Build modes: debug, release, lto, instrument
- `make bear` support that emits `compile_commands.json` via clang `-MJ` fragments
- Submodule setup and pin verification scripts for `third_party/cpp-core`
- UI types: `plain`, `ncurses`, `imgui`
- ImGui backends: `glfw_vulkan`, `glfw_opengl3`

## Prerequisites

- clang++ with C++26 module support
- make
- git
- cargo-generate 0.23+
- for `ui_type=ncurses`: ncurses development package
- for `ui_type=imgui`: GLFW development package
- for `ui_type=imgui` and `imgui_backend=glfw_vulkan`: Vulkan loader development package (`-lvulkan`)
- for `ui_type=imgui` and `imgui_backend=glfw_opengl3`: desktop OpenGL development package (`-lGL` on Linux)

## Usage

Generate a project from this repository payload folder:

```sh
cargo generate --git https://github.com/efvincent/cpp-template.git --name my-cpp26-app --define project_description='My C++26 app' --define ui_type='plain' --define imgui_backend='glfw_vulkan' template
```

Generate from a local checkout while iterating:

```sh
cargo generate --path ./template --name my-cpp26-app --define project_description='My C++26 app' --define ui_type='plain' --define imgui_backend='glfw_vulkan'
```

Generate an ncurses variant:

```sh
cargo generate --git https://github.com/efvincent/cpp-template.git --name my-cpp26-ncurses --define project_description='ncurses app' --define ui_type='ncurses' --define imgui_backend='glfw_vulkan' template
```

Generate an ImGui Vulkan variant:

```sh
cargo generate --git https://github.com/efvincent/cpp-template.git --name my-cpp26-imgui-vk --define project_description='imgui app' --define ui_type='imgui' --define imgui_backend='glfw_vulkan' template
```

Generate an ImGui OpenGL3 variant:

```sh
cargo generate --git https://github.com/efvincent/cpp-template.git --name my-cpp26-imgui-gl --define project_description='imgui app' --define ui_type='imgui' --define imgui_backend='glfw_opengl3' template
```

The ImGui Vulkan variant uses a real GLFW + Vulkan backend. On first build it bootstraps:
- `third_party/imgui`
- `third_party/Vulkan-Headers`

The ImGui OpenGL3 variant bootstraps:
- `third_party/imgui`

If link fails with `cannot find -lvulkan`, install your distro Vulkan loader development package.

Then in the generated project:

```sh
make
make release
make lto
make instrument
make bear
make verify-submodule-pin
```

## How it avoids copying repository docs

This repository keeps template payload files in `template/`.

Use the positional `template` subfolder argument for git-based generation or `--path ./template` for local generation. That ensures repository-level files like this README, CONTRIBUTING, and LICENSE are not copied into generated projects.

## Repository layout

- `cargo-generate.toml`: cargo-generate config and placeholders
- `template/`: files that are copied into generated projects
- `CONTRIBUTING.md`: contribution workflow for this template repository
- `LICENSE`: MIT license for this template repository
