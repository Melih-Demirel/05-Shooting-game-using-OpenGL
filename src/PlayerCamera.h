#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

enum PlayerActions { LEFT, RIGHT, FORWARD, BACKWARDS };

// Berekent worden gedaan a.d.h van Euler Angles, VARIABLES van Euler:
// + default waarden

const float movementSpeedPlayer_c = 0.5f;
const float mouseSensitivity_c = 0.1f;


class PlayerCamera
{
public:
	PlayerCamera();

	void processKeys(PlayerActions action, float time);
	void processMouse(float offsetX, float offsetY, bool PitchBeperken = true);
	void printInfo() { std::cout << "X: " << playerPosition.x << " Y: " << playerPosition.y << " Z: " << playerPosition.z << std::endl; }
	glm::mat4 getViewMatrix();

	float getYaw() { return yaw; }
	float getPitch() { return -pitch; }

	glm::vec3 playerPosition;
	glm::vec3 playerFront;
	glm::vec3 playerUp;
	glm::vec3 playerRight;
	glm::vec3 playerWorldUp;

	float yaw;
	float pitch;

	float movementSpeedPlayer;
	float mouseSensitivity;

private: 
	void calculateVectors();
};

