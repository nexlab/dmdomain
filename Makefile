PYINCLUDE=$(shell python-config --includes)
PYINSTALL_PATH=$(shell python -c 'import sys; print [ i for i in sys.path if i.endswith("-packages") ][0]')
MYSQL_LIB=$(shell mysql_config --cflags --libs)
MYSQL_PLUGINDIR=$(shell mysql_config --plugindir)

command:
	gcc -Wall -o dmdomain DMDomain.c

all: lib python command 

lib:
	gcc -Wall -fPIC -c -o libdmdomain.o DMDomain.c
	gcc -Wall -shared -o libdmdomain.so libdmdomain.o

pymodulegen:
	python modulegen.py > _DMDomain.c
	patch -p0 < _DMDomain_fix.patch

python: lib
	gcc -Wall -fPIC ${PYINCLUDE} -c -o _DMDomain.o _DMDomain.c
	gcc -Wall -shared -o _DMDomain.so -L. -ldmdomain _DMDomain.o

mysql: lib
	gcc -Wall ${MYSQL_LIB} -ldmdomain -fPIC -shared -o mysql_udf_dmdomain.so mysql_udf_dmdomain.c

mysql_install: lib_install
	install -m 0644 mysql_udf_dmdomain.so ${MYSQL_PLUGINDIR}

lib_install:
	install -m 0644 libdmdomain.so /usr/lib
	ldconfig

install: lib_install
	install -m 0755 dmdomain /usr/bin/
	mkdir -p ${PYINSTALL_PATH}/DMDomain
	install -m 0644 _DMDomain.so ${PYINSTALL_PATH}/DMDomain/_DMDomain.so
	install -m 0644 py/* ${PYINSTALL_PATH}/DMDomain/

debug:
	gcc -Wall -DDEBUG -o dmdomain DMDomain.c


clean:
	rm -f dmdomain
	rm -f libdmdomain*
	rm -f _DMDomain.o
	rm -f *.so


