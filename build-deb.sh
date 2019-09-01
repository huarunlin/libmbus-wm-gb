#!/bin/sh
#

if [ ! -f Makefile ]; then
	#
	# regenerate automake files
	#
    echo "Running autotools..."

    autoheader \
        && aclocal \
        && libtoolize --ltdl --copy --force \
        && automake --add-missing --copy \
        && autoconf
fi

debuild -i -us -uc -b 
#sudo pbuilder build $(NAME)_$(VERSION)-1.dsc
