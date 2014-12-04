#!/usr/bin/env bash
set -e

PCEJS_DIR=$(git rev-parse --show-toplevel)

PCEJS_ARCH=$1
if [[ -z $PCEJS_ARCH ]]
  then
    echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
    exit 1
fi

echo "building pcejs-$PCEJS_ARCH"
echo "do you want to use the build from the dist/ directory?"
echo "otherwise existing build will be used"
read -n 1 -r -p "y/n "
echo
if [[ $REPLY =~ ^[Yy]$ ]]
then  
  (
    cd $PCEJS_DIR
    grunt module:${PCEJS_ARCH}
  )
  if [[ $PCEJS_ARCH != "atarist" ]]
  then
    cp $PCEJS_DIR/dist/data/${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom $PCEJS_DIR/commonjs/pcejs-${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom
  fi
else
  if [[ ! -e "$PCEJS_DIR/commonjs/pcejs-${PCEJS_ARCH}/lib/pcejs-${PCEJS_ARCH}.js" ]]
  then
    echo "existing build not found"
    exit 1
  else
    echo "using existing build"
  fi
fi
