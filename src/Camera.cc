#include "Camera.hpp"

Camera::Camera(const vec3 &pos) : position(pos) {
    updateCameraVectors();
}

mat4 Camera::ViewMat() const {
    return glm::lookAt(position, position + front, up);
}

void Camera::ProcessKeyBoard(Movement move, float deltaTime) {
    vec3 movVec;
    float movDis = deltaTime * moveSpeed;
    switch (move) {
    case FORWARD:
        movVec = front * movDis;
        break;
    case BACKWARD:
        movVec = front * (-movDis);
        break;
    case LEFT:
        movVec = -right * movDis;
        break;
    case RIGHT:
        movVec = right * (movDis);
        break;
    default:
        break;
    }
    position += movVec;
}

void Camera::ProcessCursorPos(float offsetX, float offsetY) {
    yaw += offsetX * turnSpeed;
    pitch += offsetY * turnSpeed;

    pitch = std::fmin(std::fmax(pitch, -89.0f), 89.0f);

    updateCameraVectors();
}

void Camera::ProcessScroll(float offsetY) {
    fov -= offsetY;
    fov = std::fmin(fov, 90.0f);
    fov = std::fmax(fov, 1.0f);
}

void Camera::updateCameraVectors() {
    front.y = sin(radians(pitch));
    front.x = cos(radians(pitch)) * cos(radians(yaw));
    front.z = cos(radians(pitch)) * sin(radians(yaw));

    front = normalize(front);
    right = normalize(glm::cross(front, vec3(0.0f, 1.0f, 0.0f)));
    up = normalize(glm::cross(right, front));
}
