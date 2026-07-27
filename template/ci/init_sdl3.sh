#!/usr/bin/env sh
set -eu

if ! command -v pkg-config >/dev/null 2>&1; then
  echo "pkg-config is required to detect SDL3 development files."
  echo "Install pkg-config and rerun make."
  exit 1
fi

if pkg-config --exists sdl3; then
  exit 0
fi

echo "SDL3 development package was not found via pkg-config (missing 'sdl3.pc')."
echo "Install SDL3 dev files, then verify with: pkg-config --modversion sdl3"

if [ -r /etc/os-release ]; then
  # shellcheck disable=SC1091
  . /etc/os-release
  case "${ID:-}" in
    ubuntu|debian)
      echo "Hint: sudo apt install libsdl3-dev"
      ;;
    fedora)
      echo "Hint: sudo dnf install SDL3-devel"
      ;;
    arch)
      echo "Hint: sudo pacman -S sdl3"
      ;;
    opensuse*|sles)
      echo "Hint: sudo zypper install SDL3-devel"
      ;;
    alpine)
      echo "Hint: sudo apk add sdl3-dev"
      ;;
    *)
      echo "Hint: install your distro's SDL3 development package."
      ;;
  esac
fi

exit 1