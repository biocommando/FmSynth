set CC=gcc -static

if "%1"=="tcc" (set CC=.\tcc\tcc.exe)

%CC% copy_stages.c -o ..\Data\plugins\copy_stages.exe
%CC% rand_osc_levels.c -o ..\Data\plugins\rand_osc_levels.exe
%CC% linearize.c -o ..\Data\plugins\linearize.exe
%CC% sample_wt.c -o ..\Data\plugins\sample_wt
%CC% additive_wt.c -o ..\Data\plugins\additive_wt