#include "Camera.h"
#include "../utils/OpenGL.h"
#include <cmath>

namespace alice2 {

    Camera::Camera()
        : m_projectionType(ProjectionType::Perspective)
        , m_fov(45.0f)
        , m_aspectRatio(16.0f / 9.0f)
        , m_nearPlane(0.1f)
        , m_farPlane(1000.0f)
        , m_orthoLeft(-10.0f)
        , m_orthoRight(10.0f)
        , m_orthoBottom(-10.0f)
        , m_orthoTop(10.0f)
        , m_viewDirty(true)
        , m_orbitCenter(0, 0, 0)
        , m_orbitDistance(10.0f)
        , m_orbitPitch(0.0f)
        , m_orbitYaw(0.0f)
    {
        m_transform.setPosition(Vec3(0, 0, 10));
        updateProjection();
    }

    void Camera::setPerspective(float fov, float aspect, float nearPlane, float farPlane) {
        m_projectionType = ProjectionType::Perspective;
        m_fov = fov;
        m_aspectRatio = aspect;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        updateProjection();
    }

    void Camera::setOrthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        m_projectionType = ProjectionType::Orthographic;
        m_orthoLeft = left;
        m_orthoRight = right;
        m_orthoBottom = bottom;
        m_orthoTop = top;
        m_nearPlane = nearPlane;
        m_farPlane = farPlane;
        updateProjection();
    }

    const Mat4& Camera::getViewMatrix() const {
        if (m_viewDirty) {
            updateViewMatrix();
        }
        return m_viewMatrix;
    }

    void Camera::orbit(const Vec3& center, float deltaX, float deltaY, float distance) {
        m_orbitCenter = center;
        m_orbitDistance = distance;
        
        m_orbitYaw += deltaX;
        m_orbitPitch += deltaY;
        
        // Clamp pitch to avoid gimbal lock
        m_orbitPitch = clamp(m_orbitPitch, -89.0f, 89.0f);
        
        updateOrbitPosition();
    }

    void Camera::pan(float deltaX, float deltaY) {
        Vec3 right = getRight();
        Vec3 up = getUp();
        
        Vec3 offset = right * deltaX + up * deltaY;
        m_transform.translate(offset);
        m_orbitCenter += offset;
        m_viewDirty = true;
    }

    void Camera::zoom(float delta) {
        if (m_projectionType == ProjectionType::Perspective) {
            m_fov = clamp(m_fov + delta, 1.0f, 179.0f);
            updateProjection();
        } else {
            float scale = 1.0f + delta * 0.1f;
            m_orthoLeft *= scale;
            m_orthoRight *= scale;
            m_orthoBottom *= scale;
            m_orthoTop *= scale;
            updateProjection();
        }
    }

    void Camera::dolly(float delta) {
        m_orbitDistance = std::max(0.1f, m_orbitDistance + delta);
        updateOrbitPosition();
    }

    Vec3 Camera::screenToWorldRay(float screenX, float screenY, int screenWidth, int screenHeight) const {
        // Convert screen coordinates to normalized device coordinates
        float x = (2.0f * screenX) / screenWidth - 1.0f;
        float y = 1.0f - (2.0f * screenY) / screenHeight;
        
        // Create ray in clip space
        Vec3 rayClip(x, y, -1.0f);
        
        // Transform to view space (simplified - would need proper inverse)
        Vec3 rayView = rayClip;
        rayView.z = -1.0f;
        
        // Transform to world space (simplified)
        Vec3 forward = getForward();
        Vec3 right = getRight();
        Vec3 up = getUp();
        
        Vec3 rayWorld = forward + right * rayView.x + up * rayView.y;
        return rayWorld.normalized();
    }

    Vec3 Camera::worldToScreen(const Vec3& worldPos, int screenWidth, int screenHeight) const {
        Mat4 mvp = getViewProjectionMatrix();
        Vec3 clipPos = mvp.transformPoint(worldPos);
        
        // Convert to screen coordinates
        float x = (clipPos.x + 1.0f) * 0.5f * screenWidth;
        float y = (1.0f - clipPos.y) * 0.5f * screenHeight;
        
        return Vec3(x, y, clipPos.z);
    }

    void Camera::updateProjection() {
        if (m_projectionType == ProjectionType::Perspective) {
            m_projectionMatrix = GLMatrix::perspective(m_fov, m_aspectRatio, m_nearPlane, m_farPlane);
        } else {
            // Orthographic projection matrix
            float rl = m_orthoRight - m_orthoLeft;
            float tb = m_orthoTop - m_orthoBottom;
            float fn = m_farPlane - m_nearPlane;
            
            m_projectionMatrix = Mat4();
            m_projectionMatrix.m[0] = 2.0f / rl;
            m_projectionMatrix.m[5] = 2.0f / tb;
            m_projectionMatrix.m[10] = -2.0f / fn;
            m_projectionMatrix.m[12] = -(m_orthoRight + m_orthoLeft) / rl;
            m_projectionMatrix.m[13] = -(m_orthoTop + m_orthoBottom) / tb;
            m_projectionMatrix.m[14] = -(m_farPlane + m_nearPlane) / fn;
        }
    }

    void Camera::updateOrbitPosition() {
        float pitchRad = m_orbitPitch * DEG_TO_RAD;
        float yawRad = m_orbitYaw * DEG_TO_RAD;
        
        Vec3 position;
        position.x = m_orbitCenter.x + m_orbitDistance * std::cos(pitchRad) * std::sin(yawRad);
        position.y = m_orbitCenter.y + m_orbitDistance * std::sin(pitchRad);
        position.z = m_orbitCenter.z + m_orbitDistance * std::cos(pitchRad) * std::cos(yawRad);
        
        m_transform.setPosition(position);
        m_transform.lookAt(m_orbitCenter);
        m_viewDirty = true;
    }

    void Camera::updateViewMatrix() const {
        Vec3 pos = m_transform.getPosition();
        Vec3 forward = m_transform.forward();
        Vec3 up = m_transform.up();
        
        m_viewMatrix = GLMatrix::lookAt(pos, pos + forward, up);
        m_viewDirty = false;
    }

} // namespace alice2
