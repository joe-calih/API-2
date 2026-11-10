# Studio Bus Comp

Studio Bus Comp is a stereo bus compressor VST3 plug-in built with JUCE.

## Install

1. Download and extract `StudioBusComp-Windows-VST3.zip`.
2. Copy `Studio Bus Comp.vst3` into `C:\Program Files\Common Files\VST3`, or run `Install-Windows.bat` as administrator from the extracted folder.
3. Rescan plug-ins in your DAW.

## Build on Windows

Install Visual Studio 2022 with the Desktop development with C++ workload and CMake 3.22 or newer, then run:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel
```

The VST3 bundle is generated at `build/StudioBusComp_artefacts/Release/VST3/Studio Bus Comp.vst3`.

## Downloads

Tagged commits (`v*`) are built on GitHub Actions and published as GitHub Releases. Pushes to the default branch also produce a downloadable workflow artifact.
