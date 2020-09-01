# Script to clone the forked SourceForge gawkextlib codebase, apply
# changes from the github version (by copying files), build and test,
# and prepare for merge request.

# 1. Create the codebase

rm -rf u-ctenolophon-gawkextlib
git clone ssh://ctenolophon@git.code.sf.net/u/ctenolophon/gawkextlib \
    u-ctenolophon-gawkextlib

cd u-ctenolophon-gawkextlib

# 2. Make the new default new extension

rm -rf aregex
./make_extension_directory.sh -I aregex "Cam Webb" "cw@camwebb.info"
cd aregex

# **Modify files**

# Automake/autoconf, etc
sed -i 's/-lgawkextlib/-lgawkextlib -ltre/g' Makefile.am
sed -i 's/AC_GAWK_EXTENSION/AC_GAWK_EXTENSION\n\nAC_CHECK_LIB([tre], [tre_regaexec], [],[echo "The TRE regex library is required"; exit -1])/g' configure.ac
sed -i 's/BuildRequires:    gcc/BuildRequires:    gcc\nBuildRequires:    tre-devel/g' packaging/gawk-aregex.spec.in

# Main code
cp -f ../../../aregex.c .
patch -i ../../aregex.c.patch aregex.c

# Man page
cp -f ../../../doc/aregex.3am doc

# Web page
cp -f ../../webTOC .
git add webTOC

# Test files
cp -f ../../../test/aregex.awk test
sed -i 's|./aregex|../.libs/aregex|g' test/aregex.awk
# note: the gawkextlib test suite uses gawk with --characters-as-bytes
cp -f ../../../test/aregex_b.ok test/aregex.ok

# ChangeLog
cp -f ../../../ChangeLog .

# README
cp -f ../../SF_README.md README

## **Test**

automake
autoconf
make
make check

## ** Commit **

# git commit -m "... message ..."
# git push

## ** Test in clean repo **

# D=`pwd`
# cd /tmp/
# git clone git://git.code.sf.net/u/ctenolophon/gawkextlib u-ctenolophon-gawkextlib
# cd u-ctenolophon-gawkextlib/aregex/
# autoreconf -i
# ./configure
# make
# make check
# cd ../..
# rm -rf u-ctenolophon-gawkextlib/
# cd $D

## ** Request merge **

## ** Clean up **
# rm -rf u-ctenolophon-gawkextlib
