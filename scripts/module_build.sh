#!/bin/bash
set -e
set -u

# $1 is the relative path to resolve
function resolve_path {
  node -e "process.stdout.write(require('path').resolve('$1'))"
}

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

target=$1
if [[ -z $target ]]; then
  echo "missing pcejs arch argument [macplus ibmpc atarist]" >&2
  exit 1
fi

input_dir=$(resolve_path "dist/")
commonjs_dir=$(resolve_path "commonjs/")
pkg_dir=$(resolve_path "commonjs/pcejs-${target}/")

prelude_file="${commonjs_dir}/prelude.js"
input_module_file="${input_dir}/pce-${target}.js"
epilogue_file="${commonjs_dir}/epilogue.js"
output_module_file="${pkg_dir}/lib/pcejs-${target}.js"

mkdir -p "$pkg_dir"

{
  printf "module.exports = function(deps, opts) {\n"
  cat "$prelude_file" "$input_module_file" "$epilogue_file"
  printf "\nreturn Module;\n}"
} > "$output_module_file"

# emscripten proxy-to-worker js
cleanup_file "${pkg_dir}/pce-${target}.worker.js"
copy_if_present "${input_dir}/pce-${target}.worker.js" "${pkg_dir}/pce-${target}.worker.js"

# emscripten mem init file
cleanup_file "${pkg_dir}/pce-${target}.js.mem"
copy_if_present "${input_dir}/pce-${target}.js.mem" "${pkg_dir}/pce-${target}.js.mem"

# extension rom files
copy_if_present "${input_dir}/data/${target}/${target}-pcex.rom" "${pkg_dir}/${target}-pcex.rom"
