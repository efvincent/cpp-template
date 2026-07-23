#!/usr/bin/env sh
set -eu

submodule_path="third_party/cpp-core"
submodule_url="https://github.com/efvincent/cpp-core.git"

if [ ! -d .git ]; then
  echo "Git repository not initialized. Run: git init"
  echo "Then run: git submodule add $submodule_url $submodule_path"
  exit 1
fi

mkdir -p third_party

if git submodule status "$submodule_path" >/dev/null 2>&1; then
  git submodule update --init --recursive "$submodule_path"
else
  if [ -d "$submodule_path" ] && [ -n "$(ls -A "$submodule_path" 2>/dev/null || true)" ]; then
    echo "Existing directory is present at $submodule_path but is not a registered git submodule."
    echo "Move or remove that directory, then rerun this script."
    exit 1
  fi

  git submodule add "$submodule_url" "$submodule_path"
fi
