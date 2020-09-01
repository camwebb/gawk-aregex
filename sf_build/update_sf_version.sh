# Script to update SourceForge gawkextlib codebase, apply
# changes from the github version (by copying files), build and test,
# and prepare for merge request.

# Define new version here:

VERSION=1.0.1

# Get new version of original repo
rm -rf u-ctenolophon-gawkextlib
git clone ssh://ctenolophon@git.code.sf.net/u/ctenolophon/gawkextlib \
 u-ctenolophon-gawkextlib

cd u-ctenolophon-gawkextlib/

git remote add upstream https://git.code.sf.net/p/gawkextlib/code
git fetch upstream
git checkout master
git merge -m "auto" upstream/master

cd aregex/

# Change build files:
sed -E -i "s/\ [0-9]+\.[0-9]+\.[0-9]+/ $VERSION/g" configure.ac

# Change code files
cp -f ../../../aregex.c .
patch -i ../../aregex.c.patch aregex.c

# Man page
cp -f ../../../doc/aregex.3am doc

# Web page
cp -f ../../webTOC .

# Test files
cp -f ../../../test/aregex.awk test
sed -i 's|./aregex|../.libs/aregex|g' test/aregex.awk
# note: the gawkextlib test suite uses gawk with --characters-as-bytes
cp -f ../../../test/aregex_b.ok test/aregex.ok

# ChangeLog
cp -f ../../../ChangeLog .

# NEWS
cp -f ../../SF_NEWS NEWS

# README
cp -f ../../SF_README.md README

## Build

autoreconf -i
./configure
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
