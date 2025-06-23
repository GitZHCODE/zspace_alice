#include "Camera.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace coda {

Camera::Camera()
    : m_Position(10.0f, -10.0f, 5.0f)
    , m_Target(0.0f, 0.0f, 0.0f)
    , m_Up(0.0f, 0.0f, 1.0f)
    , m_Distance(15.0f)
    , m_Azimuth(-M_PI * 0.25f)  // -45 degrees
    , m_Elevation(M_PI * 0.25f)  // 45 degrees
{
    UpdatePosition();
}

void Camera::Pan(float deltaX, float deltaY) {
    // Get camera basis vectors
    Vec3f f = (m_Target - m_Position).Normalize();
    Vec3f s = f.Cross(m_Up).Normalize();
    Vec3f u = s.Cross(f);

    // Scale pan speed with distance for more natural feel
    float factor = m_Distance * PAN_SPEED;
    
    // Calculate pan delta
    Vec3f delta = s * (-deltaX * factor) + u * (deltaY * factor);
    
    if (m_EnableDamping) {
        m_PanDelta = delta;
    } else {
        m_Target += delta;
        m_Position += delta;
    }
}

void Camera::Orbit(float deltaX, float deltaY) {
    float factor = ORBIT_SPEED * 0.5f;
    
    if (m_EnableDamping) {
        
        // Prevent elevation from crossing poles
        float newElevation = m_Elevation + deltaY * factor;
        if (newElevation > M_PI_2 || newElevation < -M_PI_2) {
            m_SphericalDelta.elevation = 0.0f;
        } else {
            m_SphericalDelta.elevation = deltaY * factor;
        }
        
        // Direct azimuth rotation without pole factor
        m_SphericalDelta.azimuth = -deltaX * factor;
        
    } else {
        // Prevent elevation from crossing poles
        float newElevation = m_Elevation + deltaY * factor;
        if (newElevation <= m_MaxPolarAngle && newElevation >= m_MinPolarAngle) {
            m_Elevation = newElevation;
        }
        
        m_Azimuth -= deltaX * factor;  // Direct azimuth rotation
        UpdatePosition();
    }
}

void Camera::Zoom(float delta) {
    // Exponential zoom for more natural feel
    float factor = std::pow(ZOOM_SPEED, std::abs(delta));
    
    if (delta < 0) {
        if (m_EnableDamping) {
            m_SphericalDelta.radius = -m_Distance * (1.0f - factor);
        } else {
            m_Distance = ClampDistance(m_Distance * factor);
            UpdatePosition();
        }
    } else {
        if (m_EnableDamping) {
            m_SphericalDelta.radius = m_Distance * (1.0f - factor);
        } else {
            m_Distance = ClampDistance(m_Distance / factor);
            UpdatePosition();
        }
    }
}

void Camera::Update() {
    if (!m_EnableDamping) return;

    // Handle pan
    m_Target += m_PanDelta;
    m_Position += m_PanDelta;
    
    // Handle orbit
    float newElevation = m_Elevation + m_SphericalDelta.elevation;
    
    // Hard clamp elevation to prevent pole crossing
    if (newElevation <= m_MaxPolarAngle && newElevation >= m_MinPolarAngle) {
        m_Elevation = newElevation;
    }
    
    m_Azimuth = NormalizeAngle(m_Azimuth + m_SphericalDelta.azimuth);
    m_Distance = ClampDistance(m_Distance + m_SphericalDelta.radius);
    
    UpdatePosition();
    
    // Decay movements
    m_PanDelta *= (1.0f - m_DampingFactor);
    m_SphericalDelta.azimuth *= (1.0f - m_DampingFactor);
    m_SphericalDelta.elevation *= (1.0f - m_DampingFactor);
    m_SphericalDelta.radius *= (1.0f - m_DampingFactor);
}

float Camera::ClampDistance(float dist) const {
    return Clamp(dist, m_MinDistance, m_MaxDistance);
}

float Camera::NormalizeAngle(float angle) const {
    angle = std::fmod(angle, 2.0f * M_PI);
    if (angle < -M_PI) angle += 2.0f * M_PI;
    if (angle > M_PI) angle -= 2.0f * M_PI;
    return angle;
}

void Camera::UpdatePosition() {
    m_Elevation = Clamp(m_Elevation, m_MinPolarAngle, m_MaxPolarAngle);
    m_Azimuth = NormalizeAngle(m_Azimuth);
    
    float sinEl = std::sin(m_Elevation);
    float cosEl = std::cos(m_Elevation);
    float sinAz = std::sin(m_Azimuth);
    float cosAz = std::cos(m_Azimuth);

    Vec3f offset(
        m_Distance * cosEl * cosAz,
        m_Distance * cosEl * sinAz,
        m_Distance * sinEl
    );

    m_Position = m_Target + offset;
}

void Camera::GetViewMatrix(float* matrix) const {
    // For Z-up coordinate system
    Vec3f f = (m_Target - m_Position).Normalize();  // Forward
    Vec3f r = f.Cross(m_Up).Normalize();           // Right
    Vec3f u = r.Cross(f);                          // Up

    // Build view matrix for Z-up system
    matrix[0] = r.x;     matrix[4] = r.y;     matrix[8] = r.z;     matrix[12] = -r.Dot(m_Position);
    matrix[1] = u.x;     matrix[5] = u.y;     matrix[9] = u.z;     matrix[13] = -u.Dot(m_Position);
    matrix[2] = -f.x;    matrix[6] = -f.y;    matrix[10] = -f.z;   matrix[14] = f.Dot(m_Position);
    matrix[3] = 0.0f;    matrix[7] = 0.0f;    matrix[11] = 0.0f;   matrix[15] = 1.0f;
}

void Camera::GetProjectionMatrix(float* matrix) const {
    float f = 1.0f / std::tan(m_Fov * 0.5f * M_PI / 180.0f);
    float nf = 1.0f / (m_Near - m_Far);

    matrix[0] = f / m_Aspect;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (m_Far + m_Near) * nf;
    matrix[11] = -1.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 2.0f * m_Far * m_Near * nf;
    matrix[15] = 0.0f;
}

} // namespace coda 