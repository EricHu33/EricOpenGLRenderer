#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

struct Plane
{
    // unit vector
    glm::vec3 normal = { 0.f, 1.f, 0.f };

    // distance from origin to the nearest point in the plane
    float distance = 0.f;
    
    Plane()
    {
    }

    Plane(const glm::vec3& p1, const glm::vec3& norm)
    {
        normal = glm::normalize(norm);
        distance = glm::dot(p1, normal);
    }
};


struct Frustum
{
    Plane topFace;
    Plane bottomFace;
    Plane rightFace;
    Plane leftFace;
    Plane farFace;
    Plane nearFace;
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Right;
    glm::vec3 Up;
    glm::vec3 WorldUp;
    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
    _movementSpeed(SPEED), _mouseSensitivity(SENSITIVITY), _zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        _yaw = yaw;
        _pitch = pitch;
        _movementSpeed = 2.5f;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix()
    {
        //return glm::lookAt(Position, Position + Front, Up);
        return GetLookAtMatrix();
    }

    
    Frustum createFrustumFromCamera(const Camera& cam, float aspect, float fovY, float zNear, float zFar)
    {
        Frustum frustum;
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * cam.Front;

        frustum.nearFace= { cam.Position + zNear * cam.Front, cam.Front };
        frustum.farFace = { cam.Position + frontMultFar, -cam.Front };
        frustum.rightFace = { cam.Position,
                                glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
        frustum.leftFace = { cam.Position,
                                glm::cross(cam.Up,frontMultFar + cam.Right * halfHSide) };
        frustum.topFace = { cam.Position,
                                glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
        frustum.bottomFace = { cam.Position,
                                glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right) };
        return frustum;
    }

    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
    {
        const auto inv = glm::inverse(proj * view);
        std::vector<glm::vec4> frustumCorners;
        
        glm::vec4 p0 = inv * glm::vec4(-1, 1, -1, 1);
        frustumCorners.push_back(p0 / p0.w);
        glm::vec4 p1 = inv * glm::vec4(1, 1, -1, 1);
        frustumCorners.push_back(p1 / p1.w);
        glm::vec4 p2 = inv * glm::vec4(-1, -1, -1, 1);
        frustumCorners.push_back(p2 / p2.w);
        glm::vec4 p3 = inv * glm::vec4(1, -1, -1, 1);
        frustumCorners.push_back(p3 / p3.w);

        glm::vec4 p4 = inv * glm::vec4(-1, 1, 1, 1);
        frustumCorners.push_back(p4 / p4.w);
        glm::vec4 p5 = inv * glm::vec4(1, 1, 1, 1);
        frustumCorners.push_back(p5 / p5.w);
        glm::vec4 p6 = inv * glm::vec4(-1, -1, 1, 1);
        frustumCorners.push_back(p6 / p6.w);
        glm::vec4 p7 = inv * glm::vec4(1, -1, 1, 1);
        frustumCorners.push_back(p7 / p7.w);
        
        return frustumCorners;
    }

    std::vector<glm::vec4> getFrustumCornersViewSpace(float hFov, int width, int height, float nearPlane, float farPlane)
    {
        std::vector<glm::vec4> frustumCorners;
        float ar = (float)width / (float)height;
        float tanHalfHFOV = tanf(glm::radians(hFov) / 2.0f);
        float tanHalfVFOV = tanf(glm::radians((hFov / ar)) / 2.0f);

        float xNear = nearPlane * tanHalfHFOV;
        float xFar = farPlane * tanHalfHFOV;
        float yNear = nearPlane * tanHalfVFOV;
        float yFar = farPlane * tanHalfVFOV;
        frustumCorners.push_back(glm::vec4(xNear, yNear, nearPlane, 1.0));
        frustumCorners.push_back(glm::vec4(-xNear, yNear, nearPlane, 1.0));
        frustumCorners.push_back(glm::vec4(xNear, -yNear, nearPlane, 1.0));
        frustumCorners.push_back(glm::vec4(-xNear, -yNear, nearPlane, 1.0));

        frustumCorners.push_back(glm::vec4(xFar, yFar, farPlane, 1.0));
        frustumCorners.push_back(glm::vec4(-xFar, yFar, farPlane, 1.0));
        frustumCorners.push_back(glm::vec4(xFar, -yFar, farPlane, 1.0));
        frustumCorners.push_back(glm::vec4(-xFar, -yFar, farPlane, 1.0));
        
        return frustumCorners;
    }

    void ProcessKeyboard(Camera_Movement movement, float deltaTime)
    {
        float cameraSpeed = static_cast<float>(_movementSpeed * deltaTime);
        if (movement == Camera_Movement::FORWARD)
            Position += cameraSpeed * Front;
        if (movement == Camera_Movement::BACKWARD)
            Position -= cameraSpeed * Front;
        if (movement == Camera_Movement::LEFT)
            Position -= glm::normalize(glm::cross(Front, Up)) * cameraSpeed;
        if (movement == Camera_Movement::RIGHT)
            Position += glm::normalize(glm::cross(Front, Up)) * cameraSpeed;
    }

    float GetFrustumWidth(float hFov, float nearPlane)
    {
        return 2.0f * std::tan(glm::radians(hFov) / 2.0f) * nearPlane;
    }

    void ProcessMouseEvent(float xOffset, float yOffset)
    {
        float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        _yaw   += xOffset;
        _pitch += yOffset;

        if(_pitch > 89.0f)
            _pitch = 89.0f;
        if(_pitch < -89.0f)
            _pitch = -89.0f;
        updateCameraVectors();
    }
private:
    float _yaw;
    float _pitch;
    float _movementSpeed;
    float _mouseSensitivity;
    float _zoom;
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::mat4 GetLookAtMatrix()
    {
        glm::mat4 translation = glm::mat4(1.0f); // Identity matrix by default
        translation[3][0] = -Position.x;
        translation[3][1] = -Position.y;
        translation[3][2] = -Position.z;

        glm::mat4 rotation = glm::mat4(1.0f); // Identity matrix by default
        rotation[0][0] = Right.x;
        rotation[1][0] = Right.y;
        rotation[2][0] = Right.z;
        rotation[0][1] = Up.x;
        rotation[1][1] = Up.y;
        rotation[2][1] = Up.z;
        rotation[0][2] = -Front.x;
        rotation[1][2] = -Front.y;
        rotation[2][2] = -Front.z;

        return rotation * translation;
    }
    
    glm::vec3 intersectPlanes(const Plane& plane1, const Plane& plane2)
    {
        glm::vec3 n1 = plane1.normal;
        glm::vec3 n2 = plane2.normal;
        return glm::cross(n1, n2);
    }
};
#endif