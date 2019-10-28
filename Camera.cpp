#include "Camera.h"

Camera::Camera(const glm::vec3 & position, const glm::vec3 & up, const float & yaw, const float & pitch) 
	: Front(glm::vec3(0.0f, 0.0f, -1.0f))
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	updateCameraVectors();
}

const glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(Position, Position + Front, Up);
}

void Camera::move(const MOVEMENT &direction, const float &distance)
{
	if (direction == FORWARD)
		Position += Front * distance;
	if (direction == BACKWARD)
		Position -= Front * distance;
	if (direction == LEFT)
		Position -= Right * distance;
	if (direction == RIGHT)
		Position += Right * distance;
}

void Camera::turn(float xoffset, float yoffset, GLboolean constrainPitch)
{
	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch) std::max(std::min(Pitch, 89.f), -89.f);

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
	xoffset *= 0.05f;
	yoffset *= 0.05f;

	Yaw += xoffset;
	Pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	// Update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors();
}

// Calculates the front vector from the Camera's (updated) Euler Angles
void Camera::updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(front);
	// Also re-calculate the Right and Up vector
	Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	Up = glm::normalize(glm::cross(Right, Front));
}