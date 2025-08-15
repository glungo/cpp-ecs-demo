# cpp-ecs-demo
Started as a simple C++ Entity Component System demo. Evolved into a modern C++ 2D game engine.
It focuses on simplicity and performance, leveraging GPU driven rendering and a robust Entity Component System (ECS) architecture.
The aim is to provide a engine capable of handling hundreds of thousands of entities and sprites being rendered at once.

## Project Structure

- `engine/core`: Core engine functionality
- `engine/entities/`: Entity Component System implementation
- `test_app/`: Demo application
- `engine-tests/`: Consolidated test suite for all engine modules

## Building the Project
Dependencies:
- CMake 3.20 or higher
- A C++ compiler supporting C++20 (e.g., GCC 10+, Clang 10+, MSVC 2019+)
- Vulkan SDK (for Windows builds)

All other dependencies will be automatically downloaded and built by CMake or the build scripts.
Currenty a solution is created for visual studio 2022, but im not maintaining it and using the cmake load in VS 20222 instead.
If you want to use the solution, know that will probably need some setup to get it working.
### Windows
Run the `build-windows.bat` script to build the project and run tests:
```
.\build-windows.bat
```
### OSX
Run the `build.sh` script to build -OSX project is heavily underdeveloped, sorry :P -
Currently I'm only supporting Windows builds, but it is planned to support OSX and Linux in the future.
```
### Test Structure
### TODO
 review the test structure, the project changed a lot since the last time I wrote this
###
All tests are now organized in the `engine-tests` directory with the following structure:
- `src/engine/`: Tests for the engine module (core)
- `src/entities/`: Tests for the entities module

Tests are consolidated into a single test executable for easier management.
