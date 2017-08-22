# 
all:    test


openssl-fips-2.0.8.tar.gz:
	wget http://www.openssl.org/source/openssl-fips-2.0.8.tar.gz


openssl-1.0.1i.tar.gz:
	wget http://www.openssl.org/source/openssl-1.0.1i.tar.gz


ssl:
	mkdir ssl


openssl-fips-2.0.8/.built:	openssl-fips-2.0.8.tar.gz ssl
	gunzip -c openssl-fips-2.0.8.tar.gz | tar xf -
	cd openssl-fips-2.0.8; \
	export FIPSDIR=$$PWD/../ssl/fips2.0; \
	./config; \
	make; \
	make install; \
	touch .built


openssl-1.0.1i/.built:	openssl-fips-2.0.8/.built openssl-1.0.1i.tar.gz
	gunzip  -c openssl-1.0.1i.tar.gz | tar xf -
	cd openssl-1.0.1i; \
	./config fips shared --openssldir=$$PWD/../ssl --with-fipsdir=$$PWD/../ssl/fips2.0; \
	make depend; \
	make; \
	make install; \
	touch .built


test:   openssl-1.0.1i/.built
	OPENSSL_FIPS=1 LD_LIBRARY_PATH=ssl/lib ssl/bin/openssl version
	# this should work:
	LD_LIBRARY_PATH=ssl/lib ssl/bin/openssl md5 Makefile
	# should get errors for this:
	OPENSSL_FIPS=1 LD_LIBRARY_PATH=ssl/lib ssl/bin/openssl md5 Makefile


clean: 
	rm -rf openssl-fips-2.0.8 openssl-1.0.1i ssl