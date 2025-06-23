#include "CameraController.h"
#include "../core/Camera.h"
#include "InputManager.h"
#include <algorithm>

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
        m_orbitYaw += deltaX * m_orbitSpeed;
        m_orbitPitch += deltaY * m_orbitSpeed * (m_invertY ? -1.0f : 1.0f);
        
        // Clamp pitch to avoid gimbal lock
        m_orbitPitch = clamp(m_orbitPitch, -89.0f, 89.0f);
        
        updateOrbitCamera();
    }

    void CameraController::pan(float deltaX, float deltaY) {
        Vec3 right = m_camera.getRight();
        Vec3 up = m_camera.getUp();
        
        Vec3 offset = right * deltaX * m_panSpeed + up * deltaY * m_panSpeed;
        
        if (m_mode == CameraMode::Orbit) {
            m_orbitCenter += offset;
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
        m_orbitDistance = std::max(0.1f, m_orbitDistance + delta);
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

        // Handle mouse dragging for orbit
        if (m_inputManager.isMouseButtonDown(MouseButton::Left)) {
            if (!m_isDragging) {
                m_isDragging = true;
                m_lastMousePos = mouse.position;
            } else {
                Vec3 delta = mouse.position - m_lastMousePos;
                orbit(delta.x * m_mouseSensitivity * 0.5f, delta.y * m_mouseSensitivity * 0.5f);
                m_lastMousePos = mouse.position;
            }
        } else {
            m_isDragging = false;
        }

        // Handle middle mouse for panning
        if (m_inputManager.isMouseButtonDown(MouseButton::Middle)) {
            Vec3 delta = mouse.delta;
            pan(-delta.x * m_panSpeed * 0.1f, delta.y * m_panSpeed * 0.1f);
        }

        // Handle right mouse for panning as well
        if (m_inputManager.isMouseButtonDown(MouseButton::Right)) {
            Vec3 delta = mouse.delta;
            pan(-delta.x * m_panSpeed * 0.1f, delta.y * m_panSpeed * 0.1f);
        }

        // Handle mouse wheel for zooming
        float wheelDelta = mouse.wheelDelta;
        if (wheelDelta != 0.0f) {
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
        float pitchRad = m_orbitPitch * DEG_TO_RAD;
        float yawRad = m_orbitYaw * DEG_TO_RAD;
        
        Vec3 position;
        position.x = m_orbitCenter.x + m_orbitDistance * std::cos(pitchRad) * std::sin(yawRad);
        position.y = m_orbitCenter.y + m_orbitDistance * std::sin(pitchRad);
        position.z = m_orbitCenter.z + m_orbitDistance * std::cos(pitchRad) * std::cos(yawRad);
        
        m_camera.setPosition(position);
        m_camera.lookAt(m_orbitCenter);
    }

    void CameraController::updateFlyCamera() {
        // Simplified - not implemented yet
    }

} // namespace alice2
