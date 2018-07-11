gcc -fPIC -c ext.c 
gcc --shared -Wl,-soname,libext.so -o libext.so ext.o 
