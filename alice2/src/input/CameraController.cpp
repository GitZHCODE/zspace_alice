#include "CameraController.h"
#include "../core/Camera.h"
#include "InputManager.h"
#include <algorithm>
#include <iostream>

// Debug logging flag - set to true to enable detailed camera logging
#define DEBUG_CAMERA_LOGGING false

namespace alice2 {

    CameraController::CameraController(Camera& camera, InputManager& inputManager)
        : m_camera(camera)
        , m_inputManager(inputManager)
        , m_mode(CameraMode::Orbit)
        , m_orbitCenter(0, 0, 0)
        , m_orbitDistance(10.0f)
        , m_orbitPitch(0.0f)
        , m_orbitYaw(0.0f)
        , m_orbitSpeed(1.0f)
        , m_panSpeed(0.01f)
        , m_zoomSpeed(1.0f)
        , m_flySpeed(5.0f)
        , m_mouseSensitivity(0.1f)
        , m_flyPitch(0.0f)
        , m_flyYaw(0.0f)
        , m_invertY(false)
        , m_isDragging(false)
        , m_lastMousePos(0, 0, 0)
    {
        // Initialize camera position for orbit mode
        updateOrbitCamera();
    }

    void CameraController::update(float deltaTime) {
        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] CameraController::update() - deltaTime=" << deltaTime << std::endl;
            std::cout << "  Mode: " << (int)m_mode << " (0=Orbit, 1=Fly, 2=Pan)" << std::endl;
            std::cout << "  Orbit center: (" << m_orbitCenter.x << ", " << m_orbitCenter.y << ", " << m_orbitCenter.z << ")" << std::endl;
            std::cout << "  Orbit distance: " << m_orbitDistance << std::endl;
            std::cout << "  Orbit pitch/yaw: " << m_orbitPitch << "/" << m_orbitYaw << std::endl;
        }

        switch (m_mode) {
            case CameraMode::Orbit:
                handleOrbitMode(deltaTime);
                break;
            case CameraMode::Fly:
                handleFlyMode(deltaTime);
                break;
            case CameraMode::Pan:
                handlePanMode(deltaTime);
                break;
        }
    }

    void CameraController::setOrbitCenter(const Vec3& center) {
        m_orbitCenter = center;
        updateOrbitCamera();
    }

    void CameraController::setOrbitDistance(float distance) {
        m_orbitDistance = std::max(0.1f, distance);
        updateOrbitCamera();
    }

    void CameraController::orbit(float deltaX, float deltaY) {
        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] orbit: deltaX=" << deltaX << " deltaY=" << deltaY << std::endl;
            std::cout << "  Before: yaw=" << m_orbitYaw << " pitch=" << m_orbitPitch << std::endl;
        }

        m_orbitYaw += deltaX * m_orbitSpeed;
        m_orbitPitch += deltaY * m_orbitSpeed * (m_invertY ? -1.0f : 1.0f);

        // Clamp pitch to avoid gimbal lock
        m_orbitPitch = clamp(m_orbitPitch, -89.0f, 89.0f);

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "  After: yaw=" << m_orbitYaw << " pitch=" << m_orbitPitch << std::endl;
        }

        updateOrbitCamera();
    }

    void CameraController::pan(float deltaX, float deltaY) {
        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] pan: deltaX=" << deltaX << " deltaY=" << deltaY << std::endl;
            std::cout << "  Before orbit center: (" << m_orbitCenter.x << ", " << m_orbitCenter.y << ", " << m_orbitCenter.z << ")" << std::endl;
        }

        Vec3 right = m_camera.getRight();
        Vec3 up = m_camera.getUp();

        Vec3 offset = right * deltaX * m_panSpeed + up * deltaY * m_panSpeed;

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "  Pan offset: (" << offset.x << ", " << offset.y << ", " << offset.z << ")" << std::endl;
        }

        if (m_mode == CameraMode::Orbit) {
            m_orbitCenter += offset;
            if (DEBUG_CAMERA_LOGGING) {
                std::cout << "  After orbit center: (" << m_orbitCenter.x << ", " << m_orbitCenter.y << ", " << m_orbitCenter.z << ")" << std::endl;
            }
            updateOrbitCamera();
        } else {
            m_camera.getTransform().translate(offset);
        }
    }

    void CameraController::zoom(float delta) {
        if (m_mode == CameraMode::Orbit) {
            dolly(delta * m_zoomSpeed);
        } else {
            m_camera.zoom(delta * m_zoomSpeed);
        }
    }

    void CameraController::dolly(float delta) {
        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] dolly: delta=" << delta << " distance before=" << m_orbitDistance << std::endl;
        }

        m_orbitDistance = std::max(0.1f, m_orbitDistance + delta);

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "  Distance after=" << m_orbitDistance << std::endl;
        }

        updateOrbitCamera();
    }

    void CameraController::focusOnBounds(const Vec3& boundsMin, const Vec3& boundsMax) {
        Vec3 center = (boundsMin + boundsMax) * 0.5f;
        Vec3 size = boundsMax - boundsMin;
        float maxSize = std::max({size.x, size.y, size.z});
        
        setOrbitCenter(center);
        setOrbitDistance(maxSize * 2.0f);
    }

    void CameraController::resetToDefault() {
        m_orbitCenter = Vec3(0, 0, 0);
        m_orbitDistance = 10.0f;
        m_orbitPitch = 0.0f;
        m_orbitYaw = 0.0f;
        updateOrbitCamera();
    }

    void CameraController::handleOrbitMode(float /*deltaTime*/) {
        const MouseState& mouse = m_inputManager.getMouseState();

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] handleOrbitMode:" << std::endl;
            std::cout << "  Mouse pos: (" << mouse.position.x << ", " << mouse.position.y << ")" << std::endl;
            std::cout << "  Mouse delta: (" << mouse.delta.x << ", " << mouse.delta.y << ")" << std::endl;
            std::cout << "  Left button down: " << m_inputManager.isMouseButtonDown(MouseButton::Left) << std::endl;
            std::cout << "  Right button down: " << m_inputManager.isMouseButtonDown(MouseButton::Right) << std::endl;
            std::cout << "  Middle button down: " << m_inputManager.isMouseButtonDown(MouseButton::Middle) << std::endl;
            std::cout << "  Wheel delta: " << mouse.wheelDelta << std::endl;
            std::cout << "  Is dragging: " << m_isDragging << std::endl;
        }

        // Handle mouse dragging for orbit
        if (m_inputManager.isMouseButtonDown(MouseButton::Left)) {
            if (!m_isDragging) {
                m_isDragging = true;
                m_lastMousePos = mouse.position;
                if (DEBUG_CAMERA_LOGGING) {
                    std::cout << "  Started dragging at (" << m_lastMousePos.x << ", " << m_lastMousePos.y << ")" << std::endl;
                }
            } else {
                Vec3 delta = mouse.position - m_lastMousePos;
                if (DEBUG_CAMERA_LOGGING) {
                    std::cout << "  Orbit drag delta: (" << delta.x << ", " << delta.y << ")" << std::endl;
                }
                orbit(delta.x * m_mouseSensitivity * 0.5f, delta.y * m_mouseSensitivity * 0.5f);
                m_lastMousePos = mouse.position;
            }
        } else {
            if (m_isDragging && DEBUG_CAMERA_LOGGING) {
                std::cout << "  Stopped dragging" << std::endl;
            }
            m_isDragging = false;
        }

        // Handle middle mouse for panning
        if (m_inputManager.isMouseButtonDown(MouseButton::Middle)) {
            Vec3 delta = mouse.delta;
            if (DEBUG_CAMERA_LOGGING) {
                std::cout << "  Middle mouse pan: delta=(" << delta.x << ", " << delta.y << ")" << std::endl;
            }
            pan(-delta.x * m_panSpeed * 0.1f, delta.y * m_panSpeed * 0.1f);
        }

        // Handle right mouse for panning as well
        if (m_inputManager.isMouseButtonDown(MouseButton::Right)) {
            Vec3 delta = mouse.delta;
            if (DEBUG_CAMERA_LOGGING) {
                std::cout << "  Right mouse pan: delta=(" << delta.x << ", " << delta.y << ")" << std::endl;
            }
            pan(-delta.x * m_panSpeed * 0.1f, delta.y * m_panSpeed * 0.1f);
        }

        // Handle mouse wheel for zooming
        float wheelDelta = mouse.wheelDelta;
        if (wheelDelta != 0.0f) {
            if (DEBUG_CAMERA_LOGGING) {
                std::cout << "  Mouse wheel zoom: delta=" << wheelDelta << std::endl;
            }
            dolly(-wheelDelta * m_zoomSpeed);
        }
    }

    void CameraController::handleFlyMode(float /*deltaTime*/) {
        // Simplified fly mode - just basic orbit for now
        handleOrbitMode(0.0f);
    }

    void CameraController::handlePanMode(float /*deltaTime*/) {
        const MouseState& mouse = m_inputManager.getMouseState();
        
        // Handle mouse dragging for panning
        if (m_inputManager.isMouseButtonDown(MouseButton::Left)) {
            Vec3 delta = mouse.delta;
            pan(-delta.x * m_panSpeed, delta.y * m_panSpeed);
        }
        
        // Handle mouse wheel for zooming
        float wheelDelta = mouse.wheelDelta;
        if (wheelDelta != 0.0f) {
            zoom(-wheelDelta * m_zoomSpeed);
        }
    }

    void CameraController::updateOrbitCamera() {
        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "[CAMERA] updateOrbitCamera:" << std::endl;
            std::cout << "  Orbit center: (" << m_orbitCenter.x << ", " << m_orbitCenter.y << ", " << m_orbitCenter.z << ")" << std::endl;
            std::cout << "  Orbit distance: " << m_orbitDistance << std::endl;
            std::cout << "  Pitch/Yaw: " << m_orbitPitch << "/" << m_orbitYaw << std::endl;
        }

        float pitchRad = m_orbitPitch * DEG_TO_RAD;
        float yawRad = m_orbitYaw * DEG_TO_RAD;

        Vec3 position;
        position.x = m_orbitCenter.x + m_orbitDistance * std::cos(pitchRad) * std::sin(yawRad);
        position.y = m_orbitCenter.y + m_orbitDistance * std::sin(pitchRad);
        position.z = m_orbitCenter.z + m_orbitDistance * std::cos(pitchRad) * std::cos(yawRad);

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "  Calculated position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }

        m_camera.setPosition(position);
        m_camera.lookAt(m_orbitCenter);

        if (DEBUG_CAMERA_LOGGING) {
            std::cout << "  Camera updated" << std::endl;
        }
    }

    void CameraController::updateFlyCamera() {
        // Simplified - not implemented yet
    }

} // namespace alice2
