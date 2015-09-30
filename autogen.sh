#! /bin/sh
#
# autogen.sh
#
# Run this script to re-generate all maintainer-generated files.
#

here=`pwd`
cd `dirname $0`

aclocal
libtoolize --copy --force --automake 
autoheader
automake --add-missing
autoconf

echo "Now run ./configure --enable-maintainer-mode --enable-warnings"
