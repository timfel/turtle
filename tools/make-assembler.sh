#! /bin/sh

../turtle/turtle -p../crawl $1
gcc -I.. -I../crawl -S -O2 `basename $1 .t`.c
