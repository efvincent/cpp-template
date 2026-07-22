#!/usr/bin/env sh
set -eu

imgui_dir="third_party/imgui"
imgui_url="https://github.com/ocornut/imgui.git"

if [ -f "$imgui_dir/imgui.h" ]; then
  exit 0
fi

mkdir -p third_party

git clone --depth 1 "$imgui_url" "$imgui_dir"
