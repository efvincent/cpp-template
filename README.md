# C++26 Core Project Template

A standalone cargo-generate template repository for bootstrapping C++26 module projects that use the same makefile techniques as aoc-cpp and pull cpp-core as a submodule dependency.

## What this template generates

Generated projects include:
- C++26 module-first structure (`src/*.cppm` + `src/main.cpp`)
- Dynamic module discovery for both project modules and cpp-core modules
- Build modes: debug, release, lto, instrument
- `make bear` support that emits `compile_commands.json` via clang `-MJ` fragments
- Submodule setup and pin verification scripts for `third_party/cpp-core`

## Prerequisites

- clang++ with C++26 module support
- make
- git
- cargo-generate 0.23+

## Usage

Generate a project from this repository payload folder:

```sh
cargo generate --git https://github.com/efvincent/cpp-template.git --subfolder template --name my-cpp26-app
```

Generate from a local checkout while iterating:

```sh
cargo generate --path ./template --name my-cpp26-app
```

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

Use `--subfolder template` for git-based generation or `--path ./template` for local generation. That ensures repository-level files like this README, CONTRIBUTING, and LICENSE are not copied into generated projects.

## Repository layout

- `cargo-generate.toml`: cargo-generate config and placeholders
- `template/`: files that are copied into generated projects
- `CONTRIBUTING.md`: contribution workflow for this template repository
- `LICENSE`: MIT license for this template repository
