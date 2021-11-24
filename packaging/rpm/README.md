## Make dist
    ./bootstrap.sh
    ./configure
    make
    make check
    make dist

## Install rpmdevltools

    rpmdev-setuptree 
    cp sentrypeer-*.tar.gz ~/rpmbuild/SOURCES/
    rpmbuild -ba ./packaging/rpm/sentrypeer.spec
    rpmlint rpm_file
