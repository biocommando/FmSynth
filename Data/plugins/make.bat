set CC=gcc -static -Wall

if "%1"=="tcc" (set CC=.\tcc\tcc.exe)

%CC% fm_algos.c -o fm_algos.exe