// alice2 Base Sketch Template
// This is a template for creating user sketches in alice2

#include "../include/alice2.h"

using namespace alice2;

class BaseSketch : public ISketch {
public:
    BaseSketch() = default;
    ~BaseSketch() = default;

    // Sketch information
    std::string getName() const override {
        return "Base Sketch";
    }

    std::string getDescription() const override {
        return "A basic template sketch for alice2";
    }

    std::string getAuthor() const override {
        return "alice2 User";
    }

    // Sketch lifecycle
    void setup() override {
        // Initialize your sketch here
        // This is called once when the sketch is loaded
        
        // Example: Set background color
        scene().setBackgroundColor(Vec3(0.2f, 0.2f, 0.3f));
        
        // Example: Enable grid
        scene().setShowGrid(true);
        scene().setGridSize(10.0f);
        scene().setGridDivisions(10);
        
        // Example: Enable axes
        scene().setShowAxes(true);
        scene().setAxesLength(2.0f);
        
        // Example: Set camera position
        camera().setPosition(Vec3(5, 5, 5));
        camera().lookAt(Vec3(0, 0, 0));
    }

    void update(float deltaTime) override {
        // Update your sketch logic here
        // This is called every frame
        // deltaTime is the time elapsed since the last frame in seconds
        
        // Example: Rotate objects, update animations, etc.
    }

    void draw(Renderer& renderer, Camera& camera) override {
        // Draw your custom content here
        // This is called every frame after update()
        
        // Example: Draw a simple cube
        renderer.setColor(Vec3(1.0f, 0.5f, 0.2f)); // Orange color
        renderer.pushMatrix();
        renderer.drawCube(1.0f);
        renderer.popMatrix();
        
        // Example: Draw some points
        renderer.setColor(Vec3(0.2f, 1.0f, 0.5f)); // Green color
        renderer.setPointSize(5.0f);
        for (int i = 0; i < 10; i++) {
            Vec3 pos(
                std::sin(i * 0.5f) * 3.0f,
                i * 0.3f - 1.5f,
                std::cos(i * 0.5f) * 3.0f
            );
            renderer.drawPoint(pos);
        }
    }

    void cleanup() override {
        // Clean up resources here
        // This is called when the sketch is unloaded
    }

    // Input handling (optional)
    void onKeyPress(unsigned char key, int x, int y) override {
        // Handle keyboard input
        switch (key) {
            case 'r':
            case 'R':
                // Example: Reset camera
                camera().setPosition(Vec3(5, 5, 5));
                camera().lookAt(Vec3(0, 0, 0));
                break;
            case 'g':
            case 'G':
                // Example: Toggle grid
                scene().setShowGrid(!scene().getShowGrid());
                break;
            case 27: // ESC key
                // Example: Exit application
                break;
        }
    }

    void onMousePress(int button, int state, int x, int y) override {
        // Handle mouse button input
        // button: 0=left, 1=middle, 2=right
        // state: 0=down, 1=up
    }

    void onMouseMove(int x, int y) override {
        // Handle mouse movement
    }
};

// Register the sketch with alice2
ALICE2_REGISTER_SKETCH(BaseSketch)
