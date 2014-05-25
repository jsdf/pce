#!/usr/bin/env bash
set -e

PCEJS_DIR=$(git rev-parse --show-toplevel)
PCEJS_ARCH="atarist"

(
  cd $PCEJS_DIR
  grunt module:${PCEJS_ARCH}
)
