#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


class Bullet {
public:
	glm::vec4 offsett;
	glm::vec4 bulletPos;
	float afstandPerLoop = -0.1f;
	glm::mat4 model;

	float yaw;
	float pitch;
	Bullet(float yaw, float pitch, glm::vec4 bulletPos);
	
	void updatePos();
	void draw();
	
};