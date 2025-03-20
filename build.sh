

set -ex

rm -rf *.o *.so *.elf


gcc -shared -fPIC alloc-override.c heap.c llist.c -o libmyalloc.so

gcc -g -ggdb -O0 -o test.c.o -c test.c
gcc -o test.elf test.c.o -L./ -lmyalloc -Wl,-rpath=./

ldd test.elf
./test.elf
