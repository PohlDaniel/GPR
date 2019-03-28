#ifndef _SUBDIVIDE_H
#define _SUBDIVIDE_H

#include <vector>

#include "Vector.h"
#include "Extension.h"
#include "GPR.h"
#include "Linear.h"
#include "Interpolation.h"


class TriangleGrid;

class Triangle;

class Triangle{
public:


	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, bool a_subdivide);

	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
		double meanGPR1, double meanGPR2, double meanGPR3,
		double varianceGPR1, double varianceGPR2, double varianceGPR3,
		double meanLinear1, double meanLinear2, double meanLinear3,
		double varianceLinear1, double varianceLinear2, double varianceLinear3, bool a_subdivide);


	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
		double meanGPR1, double meanGPR2, double meanGPR3,
		double varianceGPR1, double varianceGPR2, double varianceGPR3,
		double meanLinear1, double meanLinear2, double meanLinear3,
		double varianceLinear1, double varianceLinear2, double varianceLinear3, bool a_subdivide, GPR* gpr);

	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
		double meanGPR1, double meanGPR2, double meanGPR3,
		double varianceGPR1, double varianceGPR2, double varianceGPR3, bool a_subdivide);

	Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
		double meanGPR1, double meanGPR2, double meanGPR3,
		double varianceGPR1, double varianceGPR2, double varianceGPR3, bool a_subdivide, GPR* gpr);


	~Triangle(){};

	Vector3f m_a;
	Vector3f m_b;
	Vector3f m_c;
	
	GPR* m_gpr;

	double m_meanGPR1, m_meanGPR2, m_meanGPR3;
	double m_varianceGPR1, m_varianceGPR2, m_varianceGPR3;
	double m_meanLinear1, m_meanLinear2, m_meanLinear3;
	double m_varianceLinear1, m_varianceLinear2, m_varianceLinear3;


	double meanError;
	double bhaError;
	double varianceError;

	bool errorCalaculated = false;
	bool subdivide = true;

	void setClockwise();

	static double area(Vector3f a, Vector3f b, Vector3f c);
	double area();
	void calcSurfaceError(TriangleGrid * a_grid, int number);

	double getMean(std::vector<double>  s);
	double getVariance(std::vector<double>  s);

	
	
};


class TriangleGrid{

public:

	
	TriangleGrid(Linear* linear, GPR* gpr, float epsilon);
	~TriangleGrid();

	std::vector<Triangle*>	m_triangles;
	Linear* m_linear;
	GPR* m_gpr;
	float m_epsilon;

	void addFacetoBuffer(Triangle* triangle, Vector3f a, Vector3f b, Vector3f c,
		double meanGPR1, double meanGPR2, double meanGPR3,
		double varianceGPR1, double varianceGPR2, double varianceGPR3,
		double meanLinear1, double meanLinear2, double meanLinear3,
		double varianceLinear1, double varianceLinear2, double varianceLinear3);

	/////////////rendering//////////////////
	void bindBuffer();
	void clearBuffer();

	unsigned int m_vertex;
	unsigned int m_meanGPR;
	unsigned int m_varianceGPR;

	std::vector<float> m_vertexBuffer;
	
	std::vector<float> m_meanBufferGPR;
	std::vector<float> m_varianceBufferGPR;

	unsigned int m_meanHybrid;
	unsigned int m_varianceHybrid;

	 
	std::vector<float> m_meanBufferHybrid;
	std::vector<float> m_varianceBufferHybrid;

	unsigned int m_colorLinear_mean;
	unsigned int m_colorLinear_variance;
	unsigned int m_meanLinear;
	unsigned int m_varianceLinear;

	std::vector<float> m_colorBufferLinear_mean;
	std::vector<float> m_colorBufferLinear_variance;

	std::vector<float> m_meanBufferLinear;
	std::vector<float> m_varianceBufferLinear;

};


#endif

