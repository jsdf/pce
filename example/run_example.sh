#!/usr/bin/env bash
set -e

PCEJS_ARCH=$1
if [[ -z $PCEJS_ARCH ]]
  then
    echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
    exit 1
fi


PCEJS_DIR=$(git rev-parse --show-toplevel)

PCEJS_MODULES_DIR="$PCEJS_DIR/commonjs"
PCEJS_ARCH_MODULE_DIR="$PCEJS_MODULES_DIR/pcejs-${PCEJS_ARCH}"
PCEJS_ARCH_EXAMPLE_DIR="$PCEJS_DIR/example/$PCEJS_ARCH"
PCEJS_NODE_BIN_DIR=$PCEJS_DIR/node_modules/.bin/

if [[ ! -e "${PCEJS_ARCH_EXAMPLE_DIR}/pce-config.cfg" ]]
  then
    echo ""
    echo "you need to download the ${PCEJS_ARCH} system to run this example "
    echo ""
    echo "grab it from: "
    echo "http://jamesfriend.com.au/pce-js/dist/${PCEJS_ARCH}-system.zip"
    echo ""
    echo "and extract it into: "
    echo "$PCEJS_ARCH_EXAMPLE_DIR"
    echo ""
    exit 1
fi

"$PCEJS_ARCH_MODULE_DIR"/prepublish.sh "$PCEJS_ARCH"

NODE_PATH="$PCEJS_MODULES_DIR:$PCEJS_DIR/node_modules" \
  "$PCEJS_NODE_BIN_DIR/browserify" "$PCEJS_ARCH_EXAMPLE_DIR/$PCEJS_ARCH.js" \
    --noparse="$PCEJS_ARCH_MODULE_DIR/lib/pcejs-${PCEJS_ARCH}.js" \
    > "$PCEJS_ARCH_EXAMPLE_DIR/bundle.js"
echo "bundle.js built"
open "http://localhost:8080/"
"$PCEJS_NODE_BIN_DIR/http-server" "$PCEJS_DIR/example/$PCEJS_ARCH"
