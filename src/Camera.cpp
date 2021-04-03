#include "Camera.h"

static const float PI = 3.141592653589f;

// Empty constructor
Camera::Camera() {

	theta = 0;
	phi = 0;

	C = glm::vec3(0.0f);
	U = glm::vec3(0.0f);
	E = glm::vec3(0.0f);

	A = glm::vec3(0.0f);
	B = glm::vec3(0.0f);
	M = glm::vec3(0.0f);
	H = glm::vec3(0.0f);
	V = glm::vec3(0.0f);

}

// Calculating A, B, H, M, and V
void Camera::setA() {
	A = glm::cross(C, U);
}
void Camera::setB() {
	B = glm::cross(A, C);
}
void Camera::setM() {
	M = E + C;
}
void Camera::setH() {
	float lenC = glm::length(C);
	float lenA = glm::length(A);
	glm::vec3 num = A * lenC;
	float radTheta = theta * (PI / 180);
	num = num * tan(radTheta);
	H = num / lenA;
	
}
void Camera::setV() {
	float lenC = glm::length(C);
	float lenB = glm::length(B);
	glm::vec3 num = B * lenC;
	float radPhi = phi * (PI / 180);
	num = num * tan(radPhi);
	V = num / lenB;
}

// Getting A, B, H, M, and V
glm::vec3 Camera::getA() {
	return A;
}
glm::vec3 Camera::getB() {
	return B;
}
glm::vec3 Camera::getM() {
	return M;
}
glm::vec3 Camera::getH() {
	return H;
}
glm::vec3 Camera::getV() {
	return V;
}

// Getting all the values
glm::vec3 Camera::getC() {
	return C;
}
glm::vec3 Camera::getU() {
	return U;
}
glm::vec3 Camera::getE() {
	return E;
}
float Camera::getTheta() {
	return theta;
}
float Camera::getPhi() {
	return phi;
}

// Setting all the values
void Camera::setC(glm::vec3 newC) {
	C = newC;
}
void Camera::setU(glm::vec3 newU) {
	U = newU;
}
void Camera::setE(glm::vec3 newE) {
	E = newE;
}
void Camera::setTheta(float newTheta) {
	theta = newTheta;
}
void Camera::setPhi(float newPhi) {
	phi = newPhi;
}