# cpp-ecs-demo
A C++ Entity Component System library

## Project Structure

- `engine/`: Core engine functionality
- `entities/`: Entity Component System implementation
- `test_app/`: Demo application
- `engine-tests/`: Consolidated test suite for all modules

## Building the Project

### Windows
Run the `build-windows.bat` script to build the project and run tests:
```
.\build-windows.bat
```

### Test Structure
All tests are now organized in the `engine-tests` directory with the following structure:
- `src/engine/`: Tests for the engine module
- `src/entities/`: Tests for the entities module

Tests are consolidated into a single test executable for easier management.
