#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

class Target {
public:
	glm::vec3 targetPos;
	glm::mat4 model;
	Target(glm::vec3 targetPoss, glm::vec3 control, glm::vec3 end) {
		this->targetPos = targetPoss;
		this->start = targetPoss;
		this->control = control;
		this->end = end;
		this->iterate = 0.0f;
	}

	// bezier
	glm::vec3 control;
	glm::vec3 control2;
	glm::vec3 end;
	glm::vec3 start;
	float iterate;
	float getPoint(float n1, float n2, float perc)
	{
		float diff = n2 - n1;

		return n1 + (diff * perc);
	}

	void bezier() {
		float za, xa, zb, xb, x, z;
		if (iterate < 1.0f) {
			xa = getPoint(start.x, control.x, iterate);
			xb = getPoint(control.x, end.x, iterate);
			za = getPoint(start.z, control.z, iterate);
			zb = getPoint(control.z, end.z, iterate);

			x = getPoint(xa, xb, iterate);
			z = getPoint(za, zb, iterate);
			targetPos.x = x;
			targetPos.z = z;
		}
		else {
			iterate = 0.0f;
			this->targetPos = start;
		}
		iterate += 0.0005f;
	}

	void bezier2control() {
		float  x, z;
		float xa, xb, xc, xd, xe;
		float za, zb, zc, zd, ze;
		if (iterate < 1.0f) {
			xa = getPoint(start.x, control.x, iterate);
			xb = getPoint(control.x, control2.x, iterate);
			xc = getPoint(control2.x, end.x, iterate);
			za = getPoint(start.z, control.z, iterate);
			zb = getPoint(control.z, control2.z, iterate);
			zc = getPoint(control2.z, end.z, iterate);
			xd = getPoint(xa, xb, iterate);
			xe = getPoint(xb, xc, iterate);
			zd = getPoint(za, zb, iterate);
			ze = getPoint(zb, zc, iterate);

			x = getPoint(xd, xe, iterate);
			z = getPoint(zd, ze, iterate);
			targetPos.x = x;
			targetPos.z = z;
		}
		else {
			iterate = 0.0f;
			this->targetPos = start;
		}
		iterate += 0.0005f;
	}

};