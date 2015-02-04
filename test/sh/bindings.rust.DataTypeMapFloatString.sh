#!/bin/sh
rustc --out-dir "${HYPERDEX_BUILDDIR}"/test/rust -o DataTypeMapFloatString "${HYPERDEX_SRCDIR}"/test/rust/DataTypeMapFloatString.rs

python2 "${HYPERDEX_SRCDIR}"/test/runner.py --space="space kv key k attributes map(float, string) v" --daemons=1 -- \
    "${HYPERDEX_BUILDDIR}"/DataTypeMapFloatString {HOST} {PORT}
