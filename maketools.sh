#!/bin/bash

set -e

export CLEANPATH=$PATH

wget ftp://ftp.gnu.org/gnu/binutils/binutils-2.28.tar.bz2
wget ftp://ftp.gnu.org/gnu/gcc/gcc-7.1.0/gcc-7.1.0.tar.bz2
sha256sum -c sha256packages
tar -vxf binutils-2.28.tar.bz2
tar -vxf gcc-7.1.0.tar.bz2

export MAKEFLAGS="-j `grep -c ^processor /proc/cpuinfo`"
export PREFIX="`pwd`/tools"
export TARGET=i386-elf
export PATH="$PREFIX/bin:$PATH"

mkdir build-binutils
cd build-binutils
../binutils-2.28/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd ../gcc-7.1.0
contrib/download_prerequisites
cd ..
mkdir build-gcc
cd build-gcc
../gcc-7.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

ln -v tools/bin/i386-elf-ar tools/bin/i386-echidnaos-ar
ln -v tools/bin/i386-elf-as tools/bin/i386-echidnaos-as
ln -v tools/bin/i386-elf-gcc tools/bin/i386-echidnaos-gcc
ln -v tools/bin/i386-elf-gcc tools/bin/i386-echidnaos-cc
ln -v tools/bin/i386-elf-ranlib tools/bin/i386-echidnaos-ranlib

wget "http://www.cpan.org/src/5.0/perl-5.20.3.tar.gz"
tar -xvf perl-5.20.3.tar.gz
export PREFIX="`pwd`/perl520"
cd perl-5.20.3
./Configure -des -Dprefix="$PREFIX"
make
make install
cd ..
export OLDPATH=$PATH
export PATH=`pwd`/perl520/bin:$PATH
mkdir autotools
wget "http://ftp.gnu.org/gnu/automake/automake-1.12.tar.gz"
wget "http://ftp.gnu.org/gnu/autoconf/autoconf-2.65.tar.gz"
tar -xvf automake-1.12.tar.gz
tar -xvf autoconf-2.65.tar.gz
export PREFIX="`pwd`/autotools"
mkdir build-automake && cd build-automake && ../automake-1.12/configure --prefix="$PREFIX" && make && make install && cd ..
mkdir build-autoconf && cd build-autoconf && ../autoconf-2.65/configure --prefix="$PREFIX" && make && make install && cd ..
export PATH=$OLDPATH
export PATH=`pwd`/autotools/bin:$PATH

wget "ftp://sourceware.org/pub/newlib/newlib-2.5.0.tar.gz"
tar -xvf newlib-2.5.0.tar.gz
cp -rv newlib-patch/* newlib-2.5.0/
pushd newlib-2.5.0/newlib/libc/sys
autoconf
popd
pushd newlib-2.5.0/newlib/libc/sys/echidnaos
autoreconf
popd

mkdir toolchain
export DESTDIR="`pwd`/toolchain"
mkdir build-newlib
cd build-newlib
../newlib-2.5.0/configure --prefix=/usr --target=i386-echidnaos
make all
make DESTDIR="$DESTDIR" install
cd ..
cp -r $DESTDIR/usr/i386-echidnaos/* $DESTDIR/usr/
unset DESTDIR

export PATH=$CLEANPATH
rm -rf build-binutils binutils-2.28 build-gcc gcc-7.1.0 autotools build-automake build-autoconf
wget "http://ftp.gnu.org/gnu/automake/automake-1.11.6.tar.gz"
wget "http://ftp.gnu.org/gnu/autoconf/autoconf-2.64.tar.gz"
tar -xvf automake-1.11.6.tar.gz
tar -xvf autoconf-2.64.tar.gz
export PATH=`pwd`/perl520/bin:$PATH
mkdir autotools
export PREFIX="`pwd`/autotools"
mkdir build-automake && cd build-automake && ../automake-1.11.6/configure --prefix="$PREFIX" && make && make install && cd ..
mkdir build-autoconf && cd build-autoconf && ../autoconf-2.64/configure --prefix="$PREFIX" && make && make install && cd ..
export PATH=$PREFIX/bin:$CLEANPATH
tar -xvf binutils-2.28.tar.bz2
cp -rv binutils-patch/* binutils-2.28/
pushd binutils-2.28/ld
automake
popd
export PREFIX="`pwd`/toolchain"
export TARGET=i386-echidnaos
export PATH=$PREFIX/bin:$CLEANPATH
mkdir build-binutils
cd build-binutils
../binutils-2.28/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$PREFIX" --disable-nls --disable-werror
make
make install
cd ..

tar -xvf gcc-7.1.0.tar.bz2
cp -rv gcc-patch/* gcc-7.1.0/
cd gcc-7.1.0 && contrib/download_prerequisites && cd ..
mkdir build-gcc
cd build-gcc
../gcc-7.1.0/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot="$PREFIX" --disable-nls --enable-languages=c --with-newlib
make
make install
cd ..











