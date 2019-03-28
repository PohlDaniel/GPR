#ifndef _MODELMATRIX_H
#define _MODELMATRIX_H

#include "Vector.h"

class ModelMatrix{

public:

	ModelMatrix();
	~ModelMatrix();

	const Matrix4f &getTransformationMatrix() const;
	const Matrix4f &getInvTransformationMatrix() const;

	void rotate(const Vector3f &axis, float degrees);
	void translate(float dx, float dy, float dz);
	void scale(float a, float b, float c);
	void setPosition(float dx, float dy, float dz);

	void rotate2(const Vector3f &axis, float degrees);
	void translate2(float dx, float dy, float dz);
	void scale2(float a, float b, float c);

	Vector3f orientation;
	Vector3f position;

private:

	Vector3f startPosition;
	bool pos;

	Matrix4f T;
	Matrix4f invT;
};


#endif