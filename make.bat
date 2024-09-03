rem @echo off
node params-compile.js
node build-info.js
g++ -c AdsrEnvelope.cpp EnvelopeStage.cpp -Ofast || exit /b

set flags=-DVST_COMPAT -DWIN32 -DWINDOWS -DUNICODE -DWINDOWS_GUI
rem g++ -c fst-extension/VSTGui/*.cpp -I"fst-extension/src/" %flags% -trigraphs || exit /b
rem g++ -c fst-extension/src/FstAudioEffect.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags% || exit /b

g++ -c PresetManager.cpp -Ofast || exit /b
g++ -c FmSynthGui.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags% || exit /b
g++ -c FmSynth.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags% -Ofast || exit /b

g++ -shared  ^
AdsrEnvelope.o FstAudioEffect.o EnvelopeStage.o FmSynth.o PresetManager.o ^
aeffguieditor.o FmSynthGui.o vstgui.o vstcontrols.o ^
-L. -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 --def FmSynth.def ^
-o 4OpFmSynth.dll -Ofast || exit /b

xcopy 4OpFmSynth.dll "C:\Program Files (x86)\VstPlugins\4OpFmSynth.dll" /Y || exit /b