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
cargo generate --path . --name smoke-template --destination /tmp
cd /tmp/smoke-template
make debug
make bear
```

Expected results:
- build succeeds
- `compile_commands.json` exists
- `third_party/cpp-core` initializes automatically on first build

## Scope rules

- Keep repository documentation at repo root.
- Keep generated-project files under `template/`.
- Do not place repository-only docs in `template/`.

## Style and compatibility

- Preserve existing makefile dynamic module discovery behavior.
- Keep scripts POSIX shell compatible.
- Keep generated output minimal and deterministic.
