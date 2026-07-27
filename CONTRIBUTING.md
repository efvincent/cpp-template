# Contributing

Thanks for contributing.

## Development workflow

1. Create a branch for your change.
2. Edit template payload files under `template/`.
3. If you change template behavior, regenerate a smoke project and build it.
4. Open a pull request with a short rationale and test evidence.

## Validation checklist

Run these checks from this repository root:

```sh
cargo generate --path ./template --name smoke-plain --define project_description='smoke' --define ui_type='plain' --destination /tmp
cargo generate --path ./template --name smoke-ncurses --define project_description='smoke' --define ui_type='ncurses' --destination /tmp
cargo generate --path ./template --name smoke-sdl3 --define project_description='smoke' --define ui_type='sdl3' --destination /tmp
cargo generate --path ./template --name smoke-imgui-vk --define project_description='smoke' --define ui_type='imgui' --define imgui_backend='glfw_vulkan' --destination /tmp
cargo generate --path ./template --name smoke-imgui-gl --define project_description='smoke' --define ui_type='imgui' --define imgui_backend='glfw_opengl3' --destination /tmp

for p in /tmp/smoke-plain /tmp/smoke-ncurses /tmp/smoke-sdl3 /tmp/smoke-imgui-vk /tmp/smoke-imgui-gl; do
	(cd "$p" && make && make bear)
done
```

Expected results:
- all variant builds succeed
- `compile_commands.json` exists in each generated project
- `third_party/cpp-core` initializes automatically on first build

CI note:
GitHub Actions runs the same matrix smoke coverage in `.github/workflows/template-smoke.yml`.

## Scope rules

- Keep repository documentation at repo root.
- Keep generated-project files under `template/`.
- Do not place repository-only docs in `template/`.

## Style and compatibility

- Preserve existing makefile dynamic module discovery behavior.
- Keep scripts POSIX shell compatible.
- Keep generated output minimal and deterministic.
