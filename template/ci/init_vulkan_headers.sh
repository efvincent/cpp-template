#!/usr/bin/env sh
set -eu

headers_dir="third_party/Vulkan-Headers"
headers_url="https://github.com/KhronosGroup/Vulkan-Headers.git"

if [ -f "$headers_dir/include/vulkan/vulkan.h" ]; then
  exit 0
fi

mkdir -p third_party

git clone --depth 1 "$headers_url" "$headers_dir"
