#!/bin/bash
libtoolize --install
aclocal
autoreconf -i
automake
./configure --prefix=/usr

