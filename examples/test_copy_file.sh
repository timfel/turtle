# /bin/sh

./copy_file ./copy_file.t copy_file.t.cp
if diff ./copy_file.t copy_file.t.cp; then
    exit 0;
else
    exit 1;
fi
