rem @echo off
node params-compile.js
node build-info.js
g++ -c AdsrEnvelope.cpp -Ofast
g++ -c EnvelopeStage.cpp -Ofast

set flags=-DVST_COMPAT -DWIN32 -DWINDOWS -DUNICODE -DWINDOWS_GUI
g++ -c fst-extension/VSTGui/*.cpp -I"fst-extension/src/" %flags% -trigraphs
g++ -c fst-extension/src/FstAudioEffect.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags%

g++ -c PresetManager.cpp -Ofast
g++ -c FmSynthGui.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags%
g++ -c FmSynth.cpp -I"fst-extension/VSTGui/" -I"fst-extension/src/" %flags% -Ofast

dllwrap  --output-def libFmSynth.def  --driver-name c++ ^
AdsrEnvelope.o FstAudioEffect.o EnvelopeStage.o FmSynth.o PresetManager.o ^
aeffguieditor.o FmSynthGui.o vstgui.o vstcontrols.o ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def FmSynth.def ^
-o 4OpFmSynth.dll -Ofast

xcopy 4OpFmSynth.dll "C:\Program Files (x86)\VstPlugins\4OpFmSynth.dll" /Y