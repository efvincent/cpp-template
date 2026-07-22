#!/usr/bin/env sh
set -eu

submodule_path="third_party/Vulkan-Headers"
submodule_url="https://github.com/KhronosGroup/Vulkan-Headers.git"
submodule_tag="vulkan-sdk-1.4.350.1"

if [ -f "$submodule_path/include/vulkan/vulkan.h" ]; then
  exit 0
fi

if [ ! -d .git ]; then
  echo "Git repository not initialized. Run: git init"
  echo "Then run: git submodule add $submodule_url $submodule_path"
  exit 1
fi

mkdir -p third_party

if git submodule status "$submodule_path" >/dev/null 2>&1; then
  git submodule update --init --recursive "$submodule_path"
else
  git submodule add "$submodule_url" "$submodule_path"
fi

# Pin newly added submodule checkouts to a known-good tag for reproducible builds.
git -C "$submodule_path" fetch --tags --force
git -C "$submodule_path" checkout "$submodule_tag"
git add "$submodule_path"
