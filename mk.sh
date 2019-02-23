clear; clear
gcc -Wall -Wpedantic -c *.c -O0
gcc *.o -o krrp -O0
./krrp "$@"
rm *.o 2> /dev/null
