# Studio Bus Comp

Studio Bus Comp is a stereo bus compressor built with JUCE. It is available as both a VST3 plug-in for DAWs and a standalone Windows application.

## Run the standalone app

Download `StudioBusComp.exe` from the [latest GitHub release](https://github.com/joe-calih/API-2/releases/latest) and run it. Connect an audio device when prompted.

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

The VST3 bundle is generated at `build/StudioBusComp_artefacts/Release/VST3/Studio Bus Comp.vst3`. The standalone executable is generated at `build/StudioBusComp_artefacts/Release/Standalone/Studio Bus Comp.exe`.

## Downloads

Tagged commits (`v*`) are built on GitHub Actions and published as GitHub Releases. Pushes to the default branch also produce a downloadable workflow artifact.
