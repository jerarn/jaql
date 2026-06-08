#!/usr/bin/env bash

set -euo pipefail

# Formats a single C++ file after an agent edit (afterFileEdit hook).
# Reads JSON from stdin with a "file_path" field.

input="$(cat)"
file_path="$(printf '%s' "${input}" | python3 -c "import sys, json; print(json.load(sys.stdin).get('file_path', ''))")"

if [[ -z "${file_path}" ]]; then
    exit 0
fi

case "${file_path}" in
    *.hpp|*.cpp) ;;
    *) exit 0 ;;
esac

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/../.." && pwd)"

"${repo_root}/scripts/format.sh" "${file_path}"
