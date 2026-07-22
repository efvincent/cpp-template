#!/usr/bin/env sh
set -eu

submodule_path="third_party/cpp-core"
submodule_url="https://github.com/efvincent/cpp-core.git"

if [ -f "$submodule_path/src/core.cppm" ]; then
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
