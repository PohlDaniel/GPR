#ifndef _LINEAR_H
#define _LINEAR_H

#include <iostream>

#include "Interpolation.h"
#include "Vector.h"

class Triangle;
class Linear : public Interpolation{

public:
	Linear();
	Linear(std::vector<Triangle*> a_triangles);
	~Linear();

	double getMean(std::vector<double>  s);
	double getVariance(std::vector<double>  s);

protected:

	void hit(Vector3f a_pos);
	

	Triangle *m_triangle;

	double m_A;
	double m_A1;
	double m_A2;
	double m_A3;

	std::vector<Triangle*> m_triangles;

};

#endif