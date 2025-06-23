#include "Application.h"
#include <iostream>
#include <chrono>

namespace alice2 {

    Application* Application::s_instance = nullptr;

    Application::Application()
        : m_running(false)
        , m_initialized(false)
        , m_windowTitle("alice2 - 3D Scene Viewer")
        , m_windowWidth(1200)
        , m_windowHeight(800)
        , m_fullscreen(false)
        , m_vsync(true)
        , m_multisampleSamples(4)
        , m_deltaTime(0.0f)
        , m_totalTime(0.0f)
        , m_lastFrameTime(0.0f)
        , m_frameCount(0)
        , m_fps(0.0f)
        , m_fpsUpdateTime(0.0f)
        , m_fpsFrameCount(0)
    {
        s_instance = this;
        
        // Create core components
        m_scene = std::make_unique<Scene>();
        m_renderer = std::make_unique<Renderer>();
        m_camera = std::make_unique<Camera>();
        m_inputManager = std::make_unique<InputManager>();
        m_cameraController = std::make_unique<CameraController>(*m_camera, *m_inputManager);
        m_sketchManager = std::make_unique<SketchManager>();
    }

    Application::~Application() {
        shutdown();
        s_instance = nullptr;
    }

    bool Application::initialize(int argc, char** argv) {
        if (m_initialized) return true;

        std::cout << "Initializing alice2..." << std::endl;

        // Initialize window and OpenGL
        if (!initializeWindow(argc, argv)) {
            std::cerr << "Failed to initialize window" << std::endl;
            return false;
        }

        if (!initializeOpenGL()) {
            std::cerr << "Failed to initialize OpenGL" << std::endl;
            return false;
        }

        // Initialize renderer
        if (!m_renderer->initialize()) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }

        // Setup camera
        m_camera->setPerspective(45.0f, (float)m_windowWidth / m_windowHeight, 0.1f, 1000.0f);
        m_camera->setPosition(Vec3(5, 5, 5));
        m_camera->lookAt(Vec3(0, 0, 0));

        // Initialize sketch manager
        m_sketchManager->initialize(m_scene.get(), m_renderer.get(), m_camera.get(), m_inputManager.get());
        m_sketchManager->scanUserSrcDirectory();

        // Setup callbacks
        setupCallbacks();

        m_initialized = true;
        std::cout << "alice2 initialized successfully" << std::endl;
        return true;
    }

    void Application::run() {
        if (!m_initialized) {
            std::cerr << "Application not initialized" << std::endl;
            return;
        }

        m_running = true;
        std::cout << "Starting alice2 main loop..." << std::endl;

        // Initialize timing
        auto startTime = std::chrono::high_resolution_clock::now();
        m_lastFrameTime = 0.0f;

        glutMainLoop();
    }

    void Application::shutdown() {
        if (!m_initialized) return;

        std::cout << "Shutting down alice2..." << std::endl;

        m_running = false;
        
        if (m_sketchManager) {
            m_sketchManager->unloadCurrentSketch();
        }
        
        if (m_renderer) {
            m_renderer->shutdown();
        }

        m_initialized = false;
    }

    bool Application::initializeWindow(int argc, char** argv) {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
        glutInitWindowSize(m_windowWidth, m_windowHeight);
        glutInitWindowPosition(100, 100);
        glutCreateWindow(m_windowTitle.c_str());

        return true;
    }

    bool Application::initializeOpenGL() {
        GLenum glewResult = glewInit();
        if (glewResult != GLEW_OK) {
            std::cerr << "GLEW initialization failed: " << glewGetErrorString(glewResult) << std::endl;
            return false;
        }

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

        return true;
    }

    void Application::setupCallbacks() {
        glutDisplayFunc(displayCallback);
        glutReshapeFunc(reshapeCallback);
        glutKeyboardFunc(keyboardCallback);
        glutMouseFunc(mouseCallback);
        glutMotionFunc(motionCallback);
        glutPassiveMotionFunc(passiveMotionCallback);
        glutTimerFunc(16, timerCallback, 0); // ~60 FPS
    }

    void Application::update() {
        updateTiming();

        // Update camera controller BEFORE resetting input states
        m_cameraController->update(m_deltaTime);

        // Update input manager (this resets mouse delta and wheel delta)
        m_inputManager->update();

        m_scene->update(m_deltaTime);

        if (m_sketchManager->hasCurrentSketch()) {
            m_sketchManager->updateCurrentSketch(m_deltaTime);
        }

        updateFPS();
    }

    void Application::render() {
        m_renderer->beginFrame();
        m_renderer->setViewport(0, 0, m_windowWidth, m_windowHeight);
        m_renderer->setCamera(*m_camera);
        
        // Set background color
        Vec3 bgColor = m_scene->getBackgroundColor();
        glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
        m_renderer->clear();
        
        // Render scene
        m_scene->render(*m_renderer, *m_camera);
        
        // Render current sketch
        if (m_sketchManager->hasCurrentSketch()) {
            m_sketchManager->drawCurrentSketch(*m_renderer, *m_camera);
        }
        
        m_renderer->endFrame();
    }

    void Application::updateTiming() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime.time_since_epoch());
        float currentTimeSeconds = duration.count() / 1000000.0f;
        
        m_deltaTime = currentTimeSeconds - m_lastFrameTime;
        m_lastFrameTime = currentTimeSeconds;
        m_totalTime += m_deltaTime;
        m_frameCount++;
    }

    void Application::updateFPS() {
        m_fpsFrameCount++;
        m_fpsUpdateTime += m_deltaTime;
        
        if (m_fpsUpdateTime >= 1.0f) {
            m_fps = m_fpsFrameCount / m_fpsUpdateTime;
            m_fpsFrameCount = 0;
            m_fpsUpdateTime = 0.0f;
        }
    }

    // Static callback implementations
    void Application::displayCallback() {
        if (s_instance) {
            s_instance->update();
            s_instance->render();
        }
    }

    void Application::reshapeCallback(int width, int height) {
        if (s_instance) {
            s_instance->m_windowWidth = width;
            s_instance->m_windowHeight = height;
            s_instance->m_camera->setAspectRatio((float)width / height);
        }
    }

    void Application::keyboardCallback(unsigned char key, int x, int y) {
        if (s_instance) {
            s_instance->m_inputManager->processKeyboard(key, x, y);

            // Handle global application keys
            switch (key) {
                case 27: // ESC key
                    s_instance->quit();
                    glutLeaveMainLoop();
                    break;
                case 'r':
                case 'R':
                    // Reset camera
                    s_instance->m_cameraController->resetToDefault();
                    break;
                case 'g':
                case 'G':
                    // Toggle grid
                    s_instance->m_scene->setShowGrid(!s_instance->m_scene->getShowGrid());
                    break;
                case 'a':
                case 'A':
                    // Toggle axes
                    s_instance->m_scene->setShowAxes(!s_instance->m_scene->getShowAxes());
                    break;
                case 'f':
                case 'F':
                    // Focus on scene bounds
                    s_instance->m_scene->calculateBounds();
                    s_instance->m_cameraController->focusOnBounds(
                        s_instance->m_scene->getBoundsMin(),
                        s_instance->m_scene->getBoundsMax()
                    );
                    break;
            }

            if (s_instance->m_sketchManager->hasCurrentSketch()) {
                s_instance->m_sketchManager->forwardKeyPress(key, x, y);
            }
        }
    }

    void Application::mouseCallback(int button, int state, int x, int y) {
        if (s_instance) {
            // Handle mouse wheel
            if (button == 3 || button == 4) { // Mouse wheel up/down
                float wheelDelta = (button == 3) ? 1.0f : -1.0f;
                s_instance->m_inputManager->processMouseWheel(wheelDelta);
            } else {
                s_instance->m_inputManager->processMouseButton(button, state, x, y);
            }

            if (s_instance->m_sketchManager->hasCurrentSketch()) {
                s_instance->m_sketchManager->forwardMousePress(button, state, x, y);
            }
        }
    }

    void Application::motionCallback(int x, int y) {
        if (s_instance) {
            s_instance->m_inputManager->processMouseMotion(x, y);
            if (s_instance->m_sketchManager->hasCurrentSketch()) {
                s_instance->m_sketchManager->forwardMouseMove(x, y);
            }
        }
    }

    void Application::passiveMotionCallback(int x, int y) {
        if (s_instance) {
            s_instance->m_inputManager->processMouseMotion(x, y);
        }
    }

    void Application::timerCallback(int value) {
        if (s_instance && s_instance->m_running) {
            glutPostRedisplay();
            glutTimerFunc(16, timerCallback, 0);
        }
    }

    // Global entry point
    int run(int argc, char** argv) {
        Application app;
        
        if (!app.initialize(argc, argv)) {
            return -1;
        }
        
        // Try to load base sketch
        app.getSketchManager().loadSketch("Base Sketch");
        
        app.run();
        return 0;
    }

} // namespace alice2
