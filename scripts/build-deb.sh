#!/usr/bin/env bash
# Build the CardputerZero .deb inside a Debian trixie arm64 container.
# Requires docker (e.g. `brew install colima docker && colima start --arch aarch64`).
# Output: dist/*.deb
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
IMAGE="fpv-checklist-cp0-build"

docker build --platform linux/arm64 -t "$IMAGE" -f "$ROOT/scripts/Dockerfile.cp0" "$ROOT/scripts"

docker run --rm --platform linux/arm64 -v "$ROOT":/src -w /src "$IMAGE" bash -c '
    set -euo pipefail
    cmake --preset cp0-native -DCM0_SDK_ROOT=/ -DCM0_ALLOW_FETCH_DEPS=ON
    cmake --build --preset cp0-native-rel
    mkdir -p dist
    cpack --config build/cp0-native/CPackConfig.cmake -G DEB -C Release -B dist
    rm -rf dist/_CPack_Packages
'

ls -1 "$ROOT"/dist/*.deb
