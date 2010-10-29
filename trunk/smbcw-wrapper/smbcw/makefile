LIBDIR ?= /usr/lib/
DESTDIR ?= 

INSTALL = /usr/bin/install -D

smbcw: smbcw.c smbcw.h smbcw_connections.c smbcw_connections.h smbcw_url.c smbcw_url.h smbcw_descriptor.c smbcw_descriptor.h
	gcc -c -fPIC smbcw.c smbcw_connections.c smbcw_url.c smbcw_descriptor.c -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -g
	gcc -shared -o libsmbcw.so smbcw.o smbcw_url.o smbcw_connections.o smbcw_descriptor.o -lsmbclient -g
#	strip libsmbcw.so

install:
	$(INSTALL) libsmbcw.so $(LIBDIR)/libsmbcw.so
	$(INSTALL) smbcw.h $(DESTDIR)/usr/include/smbcw.h

uninstall:
	rm $(LIBDIR)/libsmbcw.so -f
	rm $(DESTDIR)/usr/include/smbcw.h -f

clean:
	rm *.o -Rf
	rm *.so -Rf


