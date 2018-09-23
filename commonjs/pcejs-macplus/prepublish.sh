#!/usr/bin/env bash
set -e

PCEJS_DIR=$(git rev-parse --show-toplevel)

"$PCEJS_DIR/commonjs/prepublish.sh" "$@"
