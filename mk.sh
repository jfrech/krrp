clear; clear
clang -Wall -Wpedantic -c *.c -O0
clang *.o -o krrp -O0
./krrp "$@"
rm *.o 2> /dev/null
