#!/usr/bin/env bash
set -e

PCEJS_DIR=$(git rev-parse --show-toplevel)
PCEJS_ARCH="ibmpc"

(
  cd $PCEJS_DIR
  grunt module:${PCEJS_ARCH}
)

cp $PCEJS_DIR/dist/data/${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom $PCEJS_DIR/commonjs/pcejs-${PCEJS_ARCH}/${PCEJS_ARCH}-pcex.rom
