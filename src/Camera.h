#ifndef CAMERA_H_

#define CAMERA_H_

#include <glm/glm.hpp>
#include <stdio.h>
#include <math.h>

class Camera {
public:

	Camera();

	glm::vec3 getC();
	glm::vec3 getU();
	glm::vec3 getE();

	void setC(glm::vec3);
	void setU(glm::vec3);
	void setE(glm::vec3);

	float getTheta();
	float getPhi();

	void setTheta(float);
	void setPhi(float);

	glm::vec3 getA();
	glm::vec3 getB();
	glm::vec3 getM();
	glm::vec3 getH();
	glm::vec3 getV();

	void setA();
	void setB();
	void setM();
	void setH();
	void setV();

private:
	float theta;	// Half-angle
	float phi;		// Half-angle

	glm::vec3 C;	// Viewing vector direction
	glm::vec3 U;	// Up vector
	glm::vec3 E;	// Eye world coordinates

	glm::vec3 A;
	glm::vec3 B;
	glm::vec3 M;
	glm::vec3 H;
	glm::vec3 V;
};

#endif