#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <algorithm>

//https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h

class Camera
{
public:
	enum MOVEMENT 
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	// default yaw faces negative z
	Camera(const glm::vec3 &position = glm::vec3(0.0f, 0.0f, 0.0f), const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f), const float &yaw = -90.f, const float &pitch = 0.f);

	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	const glm::mat4 getViewMatrix() const;
	void move(const MOVEMENT &direction, const float &distance);
	void turn(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors();
};