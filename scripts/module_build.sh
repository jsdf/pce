#!/bin/bash
set -euo pipefail

arch=${1:-""}

# $1 relative path to resolve
function resolve_path {
  node -e "process.stdout.write(require('path').resolve('$1'))"
}

# $1 file to remove
cleanup_file() {
  if [ -a "$1" ]; then
    rm "$1"
  fi
}

# $1 source file
# $2 dest file
copy_if_present() {
  if [ -a "$1" ]; then
    cp "$1" "$2"
  fi
}

if [[ -z $arch ]]; then
  echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
  exit 1
fi

input_dir=$(resolve_path "dist/")
commonjs_dir=$(resolve_path "commonjs/")
pkg_dir=$(resolve_path "commonjs/pcejs-${arch}/")

prelude_file="${commonjs_dir}/prelude.js"
input_module_file="${input_dir}/pce-${arch}.js"
epilogue_file="${commonjs_dir}/epilogue.js"
output_module_file="${pkg_dir}/lib/pcejs-${arch}.js"

mkdir -p "$pkg_dir"

{
  printf "module.exports = function(deps, opts) {\n"
  cat "$prelude_file" "$input_module_file" "$epilogue_file"
  printf "\nreturn Module;\n}"
} > "$output_module_file"

# emscripten proxy-to-worker js
cleanup_file "${pkg_dir}/pce-${arch}.worker.js"
copy_if_present "${input_dir}/pce-${arch}.worker.js" "${pkg_dir}/pce-${arch}.worker.js"

# emscripten pthread js
cleanup_file "${pkg_dir}/pthread-main.js"
copy_if_present "${input_dir}/pce-${arch}.pthread-main.js" "${pkg_dir}/pthread-main.js"

# wasm
cleanup_file "${pkg_dir}/pce-${arch}.wasm"
copy_if_present "${input_dir}/pce-${arch}.wasm" "${pkg_dir}/pce-${arch}.wasm"
cleanup_file "${pkg_dir}/pce-${arch}.wasm.map"
copy_if_present "${input_dir}/pce-${arch}.wasm.map" "${pkg_dir}/pce-${arch}.wasm.map"

# emscripten mem init file
cleanup_file "${pkg_dir}/pce-${arch}.js.mem"
copy_if_present "${input_dir}/pce-${arch}.js.mem" "${pkg_dir}/pce-${arch}.js.mem"

# extension rom files
copy_if_present "${input_dir}/data/${arch}/${arch}-pcex.rom" "${pkg_dir}/${arch}-pcex.rom"
