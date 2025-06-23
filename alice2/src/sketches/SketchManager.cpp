#include "SketchManager.h"
#include "../core/Scene.h"
#include "../core/Renderer.h"
#include "../core/Camera.h"
#include "../input/InputManager.h"
#include <iostream>

namespace alice2 {

    SketchManager::SketchManager()
        : m_scene(nullptr)
        , m_renderer(nullptr)
        , m_camera(nullptr)
        , m_inputManager(nullptr)
        , m_currentSketch(nullptr)
        , m_userSrcDirectory("userSrc")
        , m_hotReloadEnabled(false)
        , m_currentLibraryHandle(nullptr)
    {
    }

    SketchManager::~SketchManager() {
        unloadCurrentSketch();
    }

    void SketchManager::initialize(Scene* scene, Renderer* renderer, Camera* camera, InputManager* inputManager) {
        m_scene = scene;
        m_renderer = renderer;
        m_camera = camera;
        m_inputManager = inputManager;
    }

    void SketchManager::scanUserSrcDirectory(const std::string& directory) {
        m_userSrcDirectory = directory;
        m_availableSketches.clear();
        
        // For now, just add a placeholder for the base sketch
        SketchInfo baseSketch;
        baseSketch.name = "Base Sketch";
        baseSketch.description = "Basic template sketch";
        baseSketch.author = "alice2";
        baseSketch.version = "1.0";
        baseSketch.filePath = directory + "/sketch_base.cpp";
        baseSketch.isLoaded = false;
        
        m_availableSketches.push_back(baseSketch);
        
        std::cout << "Found " << m_availableSketches.size() << " sketches in " << directory << std::endl;
    }

    void SketchManager::loadSketch(const std::string& name) {
        // For now, create a simple built-in sketch instead of dynamic loading
        if (name == "Base Sketch") {
            // Create a simple built-in sketch
            class SimpleSketch : public ISketch {
            public:
                std::string getName() const override { return "Built-in Base Sketch"; }
                
                void setup() override {
                    if (m_scene) {
                        m_scene->setBackgroundColor(Vec3(0.2f, 0.2f, 0.3f));
                        m_scene->setShowGrid(true);
                        m_scene->setShowAxes(true);
                    }
                }
                
                void update(float deltaTime) override {
                    // Simple update logic
                }
                
                void draw(Renderer& renderer, Camera& camera) override {
                    // Draw a simple cube
                    renderer.setColor(Vec3(1.0f, 0.5f, 0.2f));
                    renderer.pushMatrix();
                    renderer.drawCube(1.0f);
                    renderer.popMatrix();
                }
            };
            
            m_currentSketch = std::make_unique<SimpleSketch>();
            m_currentSketchName = name;
            
            // Set up the sketch with alice2 components
            m_currentSketch->setScene(m_scene);
            m_currentSketch->setRenderer(m_renderer);
            m_currentSketch->setCamera(m_camera);
            m_currentSketch->setInputManager(m_inputManager);
            
            setupCurrentSketch();
            
            std::cout << "Loaded built-in sketch: " << name << std::endl;
            
            if (m_sketchLoadedCallback) {
                m_sketchLoadedCallback(name);
            }
        } else {
            setError("Sketch not found: " + name);
        }
    }

    void SketchManager::unloadCurrentSketch() {
        if (m_currentSketch) {
            cleanupCurrentSketch();
            
            std::string sketchName = m_currentSketchName;
            m_currentSketch.reset();
            m_currentSketchName.clear();
            
            if (m_sketchUnloadedCallback) {
                m_sketchUnloadedCallback(sketchName);
            }
            
            std::cout << "Unloaded sketch: " << sketchName << std::endl;
        }
    }

    void SketchManager::reloadCurrentSketch() {
        if (!m_currentSketchName.empty()) {
            std::string name = m_currentSketchName;
            unloadCurrentSketch();
            loadSketch(name);
        }
    }

    bool SketchManager::isSketchAvailable(const std::string& name) const {
        for (const auto& sketch : m_availableSketches) {
            if (sketch.name == name) {
                return true;
            }
        }
        return false;
    }

    void SketchManager::setupCurrentSketch() {
        if (m_currentSketch) {
            try {
                m_currentSketch->setup();
            } catch (const std::exception& e) {
                setError("Error in sketch setup: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::updateCurrentSketch(float deltaTime) {
        if (m_currentSketch) {
            try {
                m_currentSketch->update(deltaTime);
            } catch (const std::exception& e) {
                setError("Error in sketch update: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::drawCurrentSketch(Renderer& renderer, Camera& camera) {
        if (m_currentSketch) {
            try {
                m_currentSketch->draw(renderer, camera);
            } catch (const std::exception& e) {
                setError("Error in sketch draw: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::cleanupCurrentSketch() {
        if (m_currentSketch) {
            try {
                m_currentSketch->cleanup();
            } catch (const std::exception& e) {
                setError("Error in sketch cleanup: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::forwardKeyPress(unsigned char key, int x, int y) {
        if (m_currentSketch) {
            try {
                m_currentSketch->onKeyPress(key, x, y);
            } catch (const std::exception& e) {
                setError("Error in sketch key press: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::forwardMousePress(int button, int state, int x, int y) {
        if (m_currentSketch) {
            try {
                m_currentSketch->onMousePress(button, state, x, y);
            } catch (const std::exception& e) {
                setError("Error in sketch mouse press: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::forwardMouseMove(int x, int y) {
        if (m_currentSketch) {
            try {
                m_currentSketch->onMouseMove(x, y);
            } catch (const std::exception& e) {
                setError("Error in sketch mouse move: " + std::string(e.what()));
            }
        }
    }

    void SketchManager::checkForChanges() {
        // Hot reload not implemented yet
    }

    void SketchManager::setError(const std::string& error) {
        m_lastError = error;
        std::cerr << "SketchManager Error: " << error << std::endl;
        
        if (m_sketchErrorCallback) {
            m_sketchErrorCallback(m_currentSketchName, error);
        }
    }

    // Platform-specific stubs (not implemented yet)
    void* SketchManager::loadLibrary(const std::string& path) {
        return nullptr;
    }

    void SketchManager::unloadLibrary(void* handle) {
        // Not implemented
    }

    void* SketchManager::getSymbol(void* handle, const std::string& name) {
        return nullptr;
    }

} // namespace alice2
