# Contributing

Thanks for contributing.

## Development workflow

1. Create a branch for your change.
2. Edit template payload files under `template/`.
3. If you change template behavior, run the full local validation checklist below.
4. Push your branch only after all local validation checks pass.
5. Open a pull request with a short rationale and local test evidence.

## Validation checklist

Run this from the repository root:

```sh
sh ci/local_validate_matrix.sh
```

If you want to inspect generated smoke projects after the run:

```sh
KEEP_SMOKE=1 sh ci/local_validate_matrix.sh
```

Manual equivalent (fallback) commands:

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

Policy note:
- local validation is the merge gate for now
- SDL CI integration is deferred to a future enhancement

## Future enhancements

- add GitHub Actions smoke coverage once SDL dependency setup is robust on hosted runners
- when CI is added, keep local validation as a mandatory pre-push check

## Scope rules

- Keep repository documentation at repo root.
- Keep generated-project files under `template/`.
- Do not place repository-only docs in `template/`.

## Style and compatibility

- Preserve existing makefile dynamic module discovery behavior.
- Keep scripts POSIX shell compatible.
- Keep generated output minimal and deterministic.
