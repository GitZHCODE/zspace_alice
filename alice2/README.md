# alice2 - 3D Scene Viewer

alice2 is a robust 3D scene viewer designed for testing and visualizing zSpace objects. Unlike traditional game engines, alice2 provides a sandbox environment where you can quickly swap different code tests and visualizations.

## Features

- **Modular Architecture**: Clean separation between Application, Scene, Renderer, and zSpace integration
- **Dynamic Sketch Loading**: Hot-swappable user sketches from the `userSrc/` directory
- **Modern 3D Rendering**: OpenGL-based rendering with camera controls
- **zSpace Integration**: Seamless integration with zSpace objects and functions
- **Cross-Platform**: Built with CMake for Windows, Linux, and macOS support

## Building

### Prerequisites

- CMake 3.16 or higher
- C++17 compatible compiler
- OpenGL
- GLEW
- FreeGLUT (or GLUT)

### Windows (Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Linux/macOS

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Quick Build Scripts

Use the provided build scripts for convenience:

- Windows: `scripts/build.bat`
- Linux/macOS: `scripts/build.sh`

## Usage

1. **Run alice2**: Execute the built `alice2` executable
2. **Create Sketches**: Add your sketches to the `userSrc/` directory
3. **Hot Reload**: alice2 will automatically detect and load new sketches

### Creating a Sketch

Create a new `.cpp` file in the `userSrc/` directory:

```cpp
#include "../include/alice2.h"

using namespace alice2;

class MySketch : public ISketch {
public:
    std::string getName() const override {
        return "My Custom Sketch";
    }

    void setup() override {
        // Initialize your sketch
        scene().setBackgroundColor(Vec3(0.1f, 0.1f, 0.2f));
        scene().setShowGrid(true);
    }

    void update(float deltaTime) override {
        // Update logic
    }

    void draw(Renderer& renderer, Camera& camera) override {
        // Custom drawing
        renderer.setColor(Vec3(1.0f, 0.5f, 0.0f));
        renderer.drawCube(1.0f);
    }
};

ALICE2_REGISTER_SKETCH(MySketch)
```

## Controls

- **Left Mouse**: Orbit camera around center
- **Middle Mouse**: Pan camera
- **Mouse Wheel**: Zoom in/out
- **R**: Reset camera position
- **G**: Toggle grid visibility
- **ESC**: Exit application

## Architecture

alice2 follows a modular architecture:

- **Application**: Main application lifecycle and window management
- **Scene**: Scene graph with hierarchical transforms and object management
- **Renderer**: OpenGL rendering pipeline with state management
- **Camera**: Perspective and orthographic camera with orbit controls
- **SketchManager**: Dynamic loading and management of user sketches
- **InputManager**: Unified input handling for keyboard and mouse

## Integration with zSpace

alice2 is designed to work seamlessly with zSpace objects:

```cpp
// Example zSpace integration (when implemented)
zSpace::zObjMesh mesh;
auto zspaceObject = std::make_shared<ZSpaceObject>(mesh);
scene().addObject(zspaceObject);
```

## License

This project is part of the zSpace alice ecosystem.
