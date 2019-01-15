# Script to integrate with SourceForge version

The Github version remains the master, but all changes can be pushed
to SF using this scripts. The cloned SF directory tree will be
deleted, but the history of changes lives at SF.

The `aregex.c.patch` patch should find all the places in `aregex.c`
where change is needed, even if the Github version has been updated.

## Files than need changing

From the file list in the message created by the
`/make_extension_directory.sh`, these files may need changing:

    aregex.c
    README
    Makefile.am
    configure.ac
    doc/aregex.3am
    packaging/gawk-aregex.spec.in   
    test/Makefile.am

The appropriate changes are made in the script (`test/Makefile.am`
needed no change)




