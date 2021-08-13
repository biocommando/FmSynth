rem @echo off
node params-compile.js
node build-info.js
g++ -c AdsrEnvelope.cpp -Ofast
g++ -c EnvelopeStage.cpp -Ofast
rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\audioeffect.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o audioeffect.o -Ofast
rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\audioeffectx.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o audioeffectx.o -Ofast
rem g++ -c "D:\VST3 SDK\public.sdk\source\vst2.x\vstplugmain.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstplugmain.o -Ofast
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\aeffguieditor.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o aeffguieditor.o
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\vstgui.cpp" -DWIN32 -trigraphs -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstgui.o
rem g++ -c "D:\VST3 SDK\vstgui.sf\vstgui\vstcontrols.cpp" -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -o vstcontrols.o

g++ -c PresetManager.cpp -Ofast
g++ -c FmSynthGui.cpp -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\vstgui.sf\vstgui" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x"
g++ -c FmSynth.cpp -DWIN32 -I"D:\VST3 SDK" -I"D:\VST3 SDK\vstgui.sf\vstgui" -I"D:\VST3 SDK\plugininterfaces\source\vst2.x" -I"D:\VST3 SDK\public.sdk\source\vst2.x" -Ofast

dllwrap  --output-def libFmSynth.def  --driver-name c++ ^
AdsrEnvelope.o audioeffect.o audioeffectx.o vstplugmain.o EnvelopeStage.o FmSynth.o PresetManager.o ^
aeffguieditor.o FmSynthGui.o vstgui.o vstcontrols.o ^
-L. --add-stdcall-alias -lole32 -lkernel32 -lgdi32 -lgdiplus -luuid -luser32 -lshell32 -mwindows --no-export-all-symbols --def FmSynth.def ^
-o 4OpFmSynth.dll -Ofast

xcopy 4OpFmSynth.dll "C:\Program Files (x86)\VstPlugins\4OpFmSynth.dll" /Y