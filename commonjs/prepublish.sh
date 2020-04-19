#!/usr/bin/env bash
set -euo pipefail

PCEJS_DIR=$(git rev-parse --show-toplevel)

arch=${1:-""}

SKIP_PROMPTS=${SKIP_PROMPTS:-""}
# SKIP_PROMPTS="y"

if [[ -z "$SKIP_PROMPTS" ]]; then
  SKIP_PROMPTS=${2:-""}
fi
if [[ -n "$SKIP_PROMPTS" ]]; then
  echo "using defaults for all prompts"
fi


if [[ -z $arch ]]
  then
    echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
    exit 1
fi

echo "building pcejs-${arch} package"
echo "do you want to update to the emulator js module output by the build system into the dist/ directory?"
echo "otherwise any emulator js module currently in module dir will be used"


UPDATE_BUILD="y"
if [[ -n $SKIP_PROMPTS ]]; then
  echo "using default response: $UPDATE_BUILD"
else
  read -n 1 -r -p "Y/n "
  echo
  if [[ $REPLY =~ ^[Nn]$ ]]; then
    UPDATE_BUILD=""
  fi
fi

if [[ -n $UPDATE_BUILD ]]; then  
  (
    cd "$PCEJS_DIR"
    ./pcejs_build module "$arch"
  )
else
  if [[ ! -e "$PCEJS_DIR/commonjs/pcejs-${arch}/lib/pcejs-${arch}.js" ]]; then
    echo "existing emulator js module not found"
    exit 1
  else
    echo "using existing emulator js module"
  fi
fi
