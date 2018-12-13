# foveated-rendering
Foveated Rendering Project Repository for Senior Design

`hardware` contains the code for hardware interfaces
`renderer` contains the code for the rendering engine

## Building
The renderer can be built by using the provided `CMakeLists.txt` 
Hardware interfaces can be built by building the source file as a DLL


## Running
Place any hardware DLLs inside a `hardware` folder next to the built exe
Place the data folder next to the exe if building in release mode, otherwise place it one folder up in debug mode.