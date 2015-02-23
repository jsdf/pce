#!/usr/bin/env bash
set -e

PCEJS_DIR=$(git rev-parse --show-toplevel)

PCEJS_ARCH=$1
if [[ -z $PCEJS_ARCH ]]
  then
    echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
    exit 1
fi

echo "building pcejs-$PCEJS_ARCH package"
echo "do you want to update to the emulator js module output by the build system into the dist/ directory?"
echo "otherwise any emulator js module currently in module dir will be used"
read -n 1 -r -p "y/n "
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then  
  (
    cd "$PCEJS_DIR"
    ./pcejs_build module "$PCEJS_ARCH"
  )
  if [[ $PCEJS_ARCH != "atarist" ]]
  then
    cp "$PCEJS_DIR/dist/data/${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom" "$PCEJS_DIR/commonjs/pcejs-${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom"
  fi
else
  if [[ ! -e "$PCEJS_DIR/commonjs/pcejs-${PCEJS_ARCH}/lib/pcejs-${PCEJS_ARCH}.js" ]]
  then
    echo "existing emulator js module not found"
    exit 1
  else
    echo "using existing emulator js module"
  fi
fi
