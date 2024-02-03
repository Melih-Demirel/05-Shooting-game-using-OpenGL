#include "PlayerCamera.h"
#include <math.h>

PlayerCamera::PlayerCamera()
{
	this->playerPosition = glm::vec3(8.90151119f, 0.5f, 3.77944779f);
	this->playerWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	this->yaw = 180.0f;
	this->pitch = 0.0f;
	this->movementSpeedPlayer = movementSpeedPlayer_c;
	this->mouseSensitivity = mouseSensitivity_c;
	this->playerFront = glm::vec3(-0.999992430f, -0.00349061913f, -0.00174172362f);
	this->playerUp = glm::vec3(-0.0296651889f, 0.999993980f, -0.00000607973f);
	this->playerRight = glm::vec3(0.00176039455f, 0.0f, -0.999998510f);
	calculateVectors();
}

void PlayerCamera::processKeys(PlayerActions action, float time)
{
	float snelheid = movementSpeedPlayer * time; 
	if (action == FORWARD)
		playerPosition += playerFront * snelheid;
	if (action == BACKWARDS)
		playerPosition -= playerFront * snelheid;
	if (action == RIGHT)
		playerPosition += playerRight * snelheid;
	if (action == LEFT)
		playerPosition -= playerRight * snelheid;

	playerPosition.y = 0.5f; //Player op z = 0 houden.
}

glm::mat4 PlayerCamera::getViewMatrix() { return glm::lookAt(playerPosition, playerPosition + playerFront, playerUp); }


void PlayerCamera::processMouse(float offsetX, float offsetY, bool PitchBeperken)
{
	this->yaw	+= offsetX * this->mouseSensitivity;

	if (yaw >= 0.0f) { yaw = fmod(yaw, 360.0f);} 
	else if (yaw < 0.0f) { yaw = fmod(yaw, -360.0f);} 
	if (yaw < 0.0f) { yaw += 360;} 

	this->pitch += offsetY * this->mouseSensitivity;

	// Als we hoger kijken dan max -> gaat de screen flippen en wij moeten dit vermijden.
	if (PitchBeperken) {
		if (this->pitch > 89.0f)
			this->pitch = 89.0f;
		if (this->pitch < -89.0f)
			this->pitch = -89.0f;
	}
	calculateVectors();
}

void PlayerCamera::calculateVectors()
{
	glm::vec3 newFront; 
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	this->playerFront = glm::normalize(newFront);
	this->playerRight = glm::normalize(glm::cross(this->playerFront, this->playerWorldUp));
	this->playerUp	  = glm::normalize(glm::cross(this->playerRight, this->playerFront));
}
