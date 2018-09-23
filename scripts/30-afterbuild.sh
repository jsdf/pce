#!/bin/bash
set -euo pipefail

echo "converting bitcode to javascript for target: ${PCEJS_TARGET}"

cleanup_file() {
  if [ -a "$1" ]; then
    rm "$1"
  fi
}

copy_if_present() {
  if [ -a "$1" ]; then
    cp "$1" "$2"
  fi
}

PCEJS_REPO_ROOT=$(git rev-parse --show-toplevel)

cp "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}" "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc"
# EMCC_DEBUG=2 
"${PCEJS_EMSDK_PATH}/emcc" "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.bc" \
  -o "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT}" \
  $PCEJS_EMFLAGS \
  $PCEJS_MAKE_CFLAGS


cp "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT}" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.${PCEJS_OUTPUT_FORMAT}"

# emscripten pthread js
cleanup_file "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.pthread-main.js"
copy_if_present "${PCEJS_PREFIX}/bin/pthread-main.js" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.pthread-main.js"

# emscripten proxy-to-worker js
cleanup_file "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.worker.js"
copy_if_present "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.worker.js" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.worker.js"

# emscripten mem init file
cleanup_file "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.js.mem"
copy_if_present "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.js.mem" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.js.mem"

# wasm
cleanup_file "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.wasm"
copy_if_present "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.wasm" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.wasm"
cleanup_file "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.wasm.map"
copy_if_present "${PCEJS_PREFIX}/bin/pce-${PCEJS_TARGET}.wasm.map" "${PCEJS_PACKAGEDIR}/pce-${PCEJS_TARGET}.wasm.map"

# extension rom files
copy_if_present "${PCEJS_PREFIX}/share/pce/${PCEJS_TARGET}/${PCEJS_TARGET}-pcex.rom" "${PCEJS_PACKAGEDIR}/data/${PCEJS_TARGET}/${PCEJS_TARGET}-pcex.rom"

