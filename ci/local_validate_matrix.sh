#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
SMOKE_ROOT=${SMOKE_ROOT:-"/tmp/cpp-template-smoke-${USER:-user}-$$"}
KEEP_SMOKE=${KEEP_SMOKE:-0}

cleanup() {
  if [ "$KEEP_SMOKE" != "1" ]; then
    rm -rf "$SMOKE_ROOT"
  fi
}

trap cleanup EXIT INT TERM

mkdir -p "$SMOKE_ROOT"

generate_variant() {
  name=$1
  shift
  cargo generate --path "$ROOT_DIR/template" --name "$name" --define project_description='smoke' "$@" --silent --destination "$SMOKE_ROOT"
}

echo "Generating smoke variants under: $SMOKE_ROOT"

generate_variant smoke-plain --define ui_type='plain'
generate_variant smoke-ncurses --define ui_type='ncurses'
generate_variant smoke-sdl3 --define ui_type='sdl3'
generate_variant smoke-imgui-vk --define ui_type='imgui' --define imgui_backend='glfw_vulkan'
generate_variant smoke-imgui-gl --define ui_type='imgui' --define imgui_backend='glfw_opengl3'

for project in smoke-plain smoke-ncurses smoke-sdl3 smoke-imgui-vk smoke-imgui-gl; do
  project_dir="$SMOKE_ROOT/$project"
  echo "Validating: $project_dir"
  (cd "$project_dir" && make -j2 && make bear && test -f compile_commands.json)
done

echo "ALL_LOCAL_VALIDATION_PASSED"
if [ "$KEEP_SMOKE" = "1" ]; then
  echo "Artifacts kept at: $SMOKE_ROOT"
fi
