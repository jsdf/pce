#!/usr/bin/env bash
set -euo pipefail

declare -a PACKAGES=("pcejs-macplus" "pcejs-ibmpc" "pcejs-atarist" "pcejs-util")
for pkgdir in "${PACKAGES[@]}"; do
  pushd "$pkgdir"
  npm version patch --no-git-tag-version
  npm publish .
  popd
done
