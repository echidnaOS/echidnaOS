#!/bin/bash

# prepare

set -e

export CLEANPATH="$PATH"
export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`"

# check if tools were built

if [ ! -f toolchain/bin/i386-echidnaos-gcc ]; then
    printf "You must build the i386-echidnaos toolchain first! (./maketools.sh)\n"
    exit 1
fi

# make sure env is clean

printf "Cleaning up\n"
rm -rf build-* autotools automake-1.12 autoconf-2.65 automake-1.11.6 autoconf-2.64 perl520 perl-5.20.3 newlib-2.5.0 binutils-2.28 gcc-7.1.0
sync

# download prerequisites

if [ ! -f automake-1.12.tar.gz ]; then
    wget "http://ftp.gnu.org/gnu/automake/automake-1.12.tar.gz"
fi
if [ ! -f autoconf-2.65.tar.gz ]; then
    wget "http://ftp.gnu.org/gnu/autoconf/autoconf-2.65.tar.gz"
fi
if [ ! -f perl-5.20.3.tar.gz ]; then
    wget "http://www.cpan.org/src/5.0/perl-5.20.3.tar.gz"
fi
if [ ! -f newlib-2.5.0.tar.gz ]; then
    wget "ftp://sourceware.org/pub/newlib/newlib-2.5.0.tar.gz"
fi

sha256sum -c sha256packages --ignore-missing

# compile perl for the autotools

tar -xvf perl-5.20.3.tar.gz
export PREFIX="`pwd`/perl520"
cd perl-5.20.3
./Configure -des -Dprefix="$PREFIX"
make
make install
cd ..
unset PREFIX

# build autotools for newlib

export PATH="`pwd`/perl520/bin:$CLEANPATH"
mkdir autotools
tar -xvf automake-1.12.tar.gz
tar -xvf autoconf-2.65.tar.gz
export PREFIX="`pwd`/autotools"
mkdir build-automake && cd build-automake && ../automake-1.12/configure --prefix="$PREFIX" && make && make install && cd ..
mkdir build-autoconf && cd build-autoconf && ../autoconf-2.65/configure --prefix="$PREFIX" && make && make install && cd ..
export PATH="$CLEANPATH"
unset PREFIX

# patch and build newlib

export PATH="`pwd`/autotools/bin:$CLEANPATH"
tar -xvf newlib-2.5.0.tar.gz
cp -rv newlib-patch/* newlib-2.5.0/
pushd newlib-2.5.0/newlib/libc/sys
autoconf
popd
pushd newlib-2.5.0/newlib/libc/sys/echidnaos
autoreconf
popd
export DESTDIR="`pwd`/toolchain"
export PATH="`pwd`/toolchain/bin:$CLEANPATH"
mkdir build-newlib
cd build-newlib
../newlib-2.5.0/configure --prefix=/usr --target=i386-echidnaos
make all
make DESTDIR="$DESTDIR" install
cd ..
cp -rv $DESTDIR/usr/i386-echidnaos/* "$DESTDIR/usr/"
unset DESTDIR
export PATH="$CLEANPATH"

# cleanup

printf "Cleaning up\n"
rm -rf build-* autotools automake-1.12 autoconf-2.65 automake-1.11.6 autoconf-2.64 perl520 perl-5.20.3 newlib-2.5.0 binutils-2.28 gcc-7.1.0
sync

printf "Done!\n"
