#!/bin/bash
set -e

rm -f sparse sparse_copy copy

make clean
make

truncate -s 1G sparse
dd if=/dev/zero of=sparse bs=1M count=1 seek=100 status=none
dd if=/dev/zero of=sparse bs=1M count=1 seek=512 status=none

echo "Before copy:"
ls -lh sparse
du -h sparse

./copy sparse sparse_copy

echo "After copy:"
ls -lh sparse_copy
du -h sparse_copy

if cmp -s sparse sparse_copy; then
    echo "✅ Files are identical"
else
    echo "❌ Files differ"
fi

