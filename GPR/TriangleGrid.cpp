#include <set>
#include <cfloat>
#include <memory>
#include <unordered_set>
#include <iostream>
#include <iterator>
#include <random>

#include "TriangleGrid.h"



void getHeatMapColor(float value, float *red, float *green, float *blue){

	const int NUM_COLORS = 3;
	static float color[NUM_COLORS][3] = { { 0, 0, 1 }, { 1, 1, 1 }, { 1, 0, 0 } };
	// A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

	int idx1;        // |-- Our desired color will be between these two indexes in "color".
	int idx2;        // |
	float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

	if (value <= 0)      { idx1 = idx2 = 0; }    // accounts for an input <=0
	else if (value >= 1)  { idx1 = idx2 = NUM_COLORS - 1; }    // accounts for an input >=0
	else
	{
		value = value * (NUM_COLORS - 1);        // Will multiply value by 3.
		idx1 = floor(value);                  // Our desired color will be after this index.
		idx2 = idx1 + 1;                        // ... and before this index (inclusive).
		fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
	}

	*red = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
	*green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
	*blue = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

void getHeatMapColor2(float value, float *red, float *green, float *blue){

	const int NUM_COLORS = 9;
	static float color[NUM_COLORS][3] = { { 69.0 / 255, 4.0 / 255, 87.0 / 255 }, { 71.0 / 255, 42.0 / 255, 122.0 / 255 }, { 60.0 / 255, 80.0 / 255, 139.0 / 255 }, { 46.0 / 255, 111.0 / 255, 142.0 / 255 }, { 34.0 / 255, 140.0 / 255, 141.0 / 255 }, { 35.0 / 255, 169.0 / 255, 131.0 / 255 }, { 78.0 / 255, 195.0 / 255, 107.0 / 255 }, { 149.0 / 255, 216.0 / 255, 64.0 / 255 }, { 248.0 / 255, 230.0 / 255, 33.0 / 255 } };
	//static float color[NUM_COLORS][3] = { { 69.0/255, 4.0/255, 87.0/255 }, { 71.0/255, 42.0/255, 122.0/255 }, { 60.0/255, 80.0/255, 139.0/255 }};

	// A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.

	int idx1;        // |-- Our desired color will be between these two indexes in "color".
	int idx2;        // |
	float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.

	if (value <= 0)      { idx1 = idx2 = 0; }    // accounts for an input <=0
	else if (value >= 1)  { idx1 = idx2 = NUM_COLORS - 1; }    // accounts for an input >=0
	else
	{
		value = value * (NUM_COLORS - 1);        // Will multiply value by 3.
		idx1 = floor(value);                  // Our desired color will be after this index.
		idx2 = idx1 + 1;                        // ... and before this index (inclusive).
		fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).
	}

	*red = (color[idx2][0] - color[idx1][0])*fractBetween + color[idx1][0];
	*green = (color[idx2][1] - color[idx1][1])*fractBetween + color[idx1][1];
	*blue = (color[idx2][2] - color[idx1][2])*fractBetween + color[idx1][2];
}

///////////////////////////////////// Triangle //////////////////////////////////////////
Triangle::Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
	double a_meanGPR1, double a_meanGPR2, double a_meanGPR3,
	double a_varianceGPR1, double a_varianceGPR2, double a_varianceGPR3, bool a_subdivide){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;



	subdivide = a_subdivide;
	m_meanGPR1 = a_meanGPR1;
	m_meanGPR2 = a_meanGPR2;
	m_meanGPR3 = a_meanGPR3;
	m_varianceGPR1 = a_varianceGPR1;
	m_varianceGPR2 = a_varianceGPR2;
	m_varianceGPR3 = a_varianceGPR3;

}


Triangle::Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
	double a_meanGPR1, double a_meanGPR2, double a_meanGPR3,
	double a_varianceGPR1, double a_varianceGPR2, double a_varianceGPR3, bool a_subdivide, GPR* a_gpr){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;

	m_gpr = a_gpr;

	subdivide = a_subdivide;
	m_meanGPR1 = a_meanGPR1;
	m_meanGPR2 = a_meanGPR2;
	m_meanGPR3 = a_meanGPR3;
	m_varianceGPR1 = a_varianceGPR1;
	m_varianceGPR2 = a_varianceGPR2;
	m_varianceGPR3 = a_varianceGPR3;

}

Triangle::Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
	double a_meanGPR1, double a_meanGPR2, double a_meanGPR3,
	double a_varianceGPR1, double a_varianceGPR2, double a_varianceGPR3,
	double a_meanLinear1, double a_meanLinear2, double a_meanLinear3,
	double a_varianceLinear1, double a_varianceLinear2, double a_varianceLinear3, bool a_subdivide){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;

	subdivide = a_subdivide;
	m_meanGPR1 = a_meanGPR1;
	m_meanGPR2 = a_meanGPR2;
	m_meanGPR3 = a_meanGPR3;
	m_varianceGPR1 = a_varianceGPR1;
	m_varianceGPR2 = a_varianceGPR2;
	m_varianceGPR3 = a_varianceGPR3;

	m_meanLinear1 = a_meanLinear1;
	m_meanLinear2 = a_meanLinear2;
	m_meanLinear3 = a_meanLinear3;
	m_varianceLinear1 = a_varianceLinear1;
	m_varianceLinear2 = a_varianceLinear2;
	m_varianceLinear3 = a_varianceLinear3;

}

Triangle::Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3,
	double a_meanGPR1, double a_meanGPR2, double a_meanGPR3,
	double a_varianceGPR1, double a_varianceGPR2, double a_varianceGPR3,
	double a_meanLinear1, double a_meanLinear2, double a_meanLinear3,
	double a_varianceLinear1, double a_varianceLinear2, double a_varianceLinear3, bool a_subdivide, GPR* a_gpr){

	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	m_gpr = a_gpr;

	subdivide = a_subdivide;
	m_meanGPR1 = a_meanGPR1;
	m_meanGPR2 = a_meanGPR2;
	m_meanGPR3 = a_meanGPR3;
	m_varianceGPR1 = a_varianceGPR1;
	m_varianceGPR2 = a_varianceGPR2;
	m_varianceGPR3 = a_varianceGPR3;

	m_meanLinear1 = a_meanLinear1;
	m_meanLinear2 = a_meanLinear2;
	m_meanLinear3 = a_meanLinear3;
	m_varianceLinear1 = a_varianceLinear1;
	m_varianceLinear2 = a_varianceLinear2;
	m_varianceLinear3 = a_varianceLinear3;

}


Triangle::Triangle(Vector3f &a_V1, Vector3f &a_V2, Vector3f &a_V3, bool a_subdivide){
	m_a = a_V1;
	m_b = a_V2;
	m_c = a_V3;
	subdivide = a_subdivide;
}

double Triangle::area(Vector3f a, Vector3f b, Vector3f c){

	return abs((a[0]*(b[2] - c[2]) + b[0]*(c[2]- a[2]) + c[0]*(a[2] - b[2])) / 2.0);
}

double Triangle::area(){

	return abs((m_a[0] * (m_b[2] - m_c[2]) + m_b[0] * (m_c[2] - m_a[2]) + m_c[0] * (m_a[2] - m_b[2])) / 2.0);
}

double Triangle::getMean(std::vector<double>  s){

	Vector3f pos = Vector3f(s[0], 0.0, s[1]);

	double A = area(m_a, m_b, m_c);
	double A1 = area(pos, m_b, m_c);
	double A2 = area(m_a, pos, m_c);
	double A3 = area(m_a, m_b, pos);

	return (A1 * m_meanGPR1 + A2 * m_meanGPR2 + A3 * m_meanGPR3) / A;
}

double Triangle::getVariance(std::vector<double>  s){

	Vector3f pos = Vector3f(s[0], 0.0, s[1]);

	double A = area(m_a, m_b, m_c);
	double A1 = area(pos, m_b, m_c);
	double A2 = area(m_a, pos, m_c);
	double A3 = area(m_a, m_b, pos);

	double alpha = A1 / A;
	double beta = A2 / A;
	double gamma = A3 / A;

	return alpha * alpha * m_varianceGPR1 + beta * beta * m_varianceGPR2 + gamma* gamma * m_varianceGPR3;
}


void Triangle::calcSurfaceError(TriangleGrid * a_grid, int number){
	
	if (errorCalaculated) return;

	std::random_device rd1;
	std::default_random_engine generator1(rd1());
	std::uniform_real_distribution<> dis1(0.0, 1.0);

	std::random_device rd2;
	std::default_random_engine generator2(rd2());
	std::uniform_real_distribution<> dis2(0.0, 1.0);

	double bha = 0;
	double mean = 0;
	double variance = 0;
	

	for (int n = 0; n < number; n++){

		double r1 = dis1(generator1);
		double r2 = dis2(generator2);

		Vector3f sample = m_a * (1 - sqrt(r1)) + m_b * sqrt(r1) * (1 - r2) + m_c * sqrt(r1) *r2;

		double mu_p = m_gpr->getMean({ sample[0], sample[2] });
		double mu_q = getMean({ sample[0], sample[2] });

		double sigma_p = m_gpr->getVariance({ sample[0], sample[2] });
		double sigma_q = getVariance({ sample[0], sample[2] });

		bha = bha + 0.25*log(0.25*(sigma_p / sigma_q + sigma_q / sigma_p + 2)) + 0.25*((mu_p - mu_q)*(mu_p - mu_q) / (sigma_p + sigma_q));
		mean = mean + abs(mu_p - mu_q);
		variance = variance + abs(sigma_p - sigma_q);
	}

	bha = bha * (area() / number);
	mean = mean  * (area() / number);
	variance = variance * (area() / number);
	
	if (mean < a_grid->m_epsilon) subdivide = false;
	
	bhaError = bha;
	meanError = mean;
	varianceError = variance;

	errorCalaculated = true;

	
}



/////////////////////////////////////////TriangleGrid///////////////////////////////////

TriangleGrid::TriangleGrid(Linear* a_linear, GPR* a_gpr, float a_epsilon){

	m_linear = a_linear;
	m_epsilon = a_epsilon;
	m_gpr = a_gpr;
}

void TriangleGrid::bindBuffer(){

	m_vertex = 0;
	glGenBuffers(1, &m_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertex);
	glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size()*sizeof(float), &m_vertexBuffer[0], GL_STATIC_DRAW);

	m_meanGPR = 0;
	glGenBuffers(1, &m_meanGPR);
	glBindBuffer(GL_ARRAY_BUFFER, m_meanGPR);
	glBufferData(GL_ARRAY_BUFFER, m_meanBufferGPR.size()*sizeof(float), &m_meanBufferGPR[0], GL_STATIC_DRAW);

	m_varianceGPR = 0;
	glGenBuffers(1, &m_varianceGPR);
	glBindBuffer(GL_ARRAY_BUFFER, m_varianceGPR);
	glBufferData(GL_ARRAY_BUFFER, m_varianceBufferGPR.size()*sizeof(float), &m_varianceBufferGPR[0], GL_STATIC_DRAW);

	m_meanHybrid = 0;
	glGenBuffers(1, &m_meanHybrid);
	glBindBuffer(GL_ARRAY_BUFFER, m_meanHybrid);
	glBufferData(GL_ARRAY_BUFFER, m_meanBufferHybrid.size()*sizeof(float), &m_meanBufferHybrid[0], GL_STATIC_DRAW);

	m_colorLinear_mean = 0;
	glGenBuffers(1, &m_colorLinear_mean);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorLinear_mean);
	glBufferData(GL_ARRAY_BUFFER, m_colorBufferLinear_mean.size()*sizeof(float), &m_colorBufferLinear_mean[0], GL_STATIC_DRAW);

	m_meanLinear = 0;
	glGenBuffers(1, &m_meanLinear);
	glBindBuffer(GL_ARRAY_BUFFER, m_meanLinear);
	glBufferData(GL_ARRAY_BUFFER, m_meanBufferLinear.size()*sizeof(float), &m_meanBufferLinear[0], GL_STATIC_DRAW);
	
	m_colorLinear_variance = 0;
	glGenBuffers(1, &m_colorLinear_variance);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorLinear_variance);
	glBufferData(GL_ARRAY_BUFFER, m_colorBufferLinear_variance.size()*sizeof(float), &m_colorBufferLinear_variance[0], GL_STATIC_DRAW);

	m_varianceLinear = 0;
	glGenBuffers(1, &m_varianceLinear);
	glBindBuffer(GL_ARRAY_BUFFER, m_varianceLinear);
	glBufferData(GL_ARRAY_BUFFER, m_varianceBufferLinear.size()*sizeof(float), &m_varianceBufferLinear[0], GL_STATIC_DRAW);


	m_varianceHybrid = 0;
	glGenBuffers(1, &m_varianceHybrid);
	glBindBuffer(GL_ARRAY_BUFFER, m_varianceHybrid);
	glBufferData(GL_ARRAY_BUFFER, m_varianceBufferHybrid.size()*sizeof(float), &m_varianceBufferHybrid[0], GL_STATIC_DRAW);

}



void TriangleGrid::addFacetoBuffer(Triangle* triangle, Vector3f a_a, Vector3f a_b, Vector3f a_c,
	double meanGPR1, double meanGPR2, double meanGPR3,
	double varianceGPR1, double varianceGPR2, double varianceGPR3,
	double meanLinear1, double meanLinear2, double meanLinear3,
	double varianceLinear1, double varianceLinear2, double varianceLinear3){

	

	float x;
	float z;

	double mean;
	double variance;

	float r, g, b;

	x = a_a[0];
	z = a_a[2];

	//mean = meanGPR1;
	//variance = varianceGPR1;

	mean = triangle->m_gpr->getMean({ x, z });
	variance = triangle->m_gpr->getVariance({ x, z });
	
	//mean = m_gpr->getMean({ x, z });
	//variance = m_gpr->getVariance({ x, z });

	m_vertexBuffer.push_back(x);
	m_vertexBuffer.push_back(0);
	m_vertexBuffer.push_back(z);

	m_meanBufferGPR.push_back(mean);
	m_varianceBufferGPR.push_back(variance);
	

	x = a_b[0];
	z = a_b[2];

	//mean = meanGPR2;
	//variance = varianceGPR2;

	mean = triangle->m_gpr->getMean({ x, z });
	variance = triangle->m_gpr->getVariance({ x, z });

	//mean = m_gpr->getMean({ x, z });
	//variance = m_gpr->getVariance({ x, z });


	m_vertexBuffer.push_back(x);
	m_vertexBuffer.push_back(0);
	m_vertexBuffer.push_back(z);

	m_meanBufferGPR.push_back(mean);
	m_varianceBufferGPR.push_back(variance);

	x = a_c[0];
	z = a_c[2];

	//mean = meanGPR3;
	//variance = varianceGPR3;

	mean = triangle->m_gpr->getMean({ x, z });
	variance = triangle->m_gpr->getVariance({ x, z });

	//mean = m_gpr->getMean({ x, z });
	//variance = m_gpr->getVariance({ x, z });

	m_vertexBuffer.push_back(x);
	m_vertexBuffer.push_back(0);
	m_vertexBuffer.push_back(z);

	m_meanBufferGPR.push_back(mean);
	m_varianceBufferGPR.push_back(variance);

	x = a_a[0];
	z = a_a[2];

	mean = meanLinear1;
	variance = varianceLinear1;

	m_meanBufferHybrid.push_back(mean);
	m_varianceBufferHybrid.push_back(variance);

	x = a_b[0];
	z = a_b[2];

	mean = meanLinear2;
	variance = varianceLinear2;

	m_meanBufferHybrid.push_back(mean);
	m_varianceBufferHybrid.push_back(variance);

	x = a_c[0];
	z = a_c[2];

	mean = meanLinear3;
	variance = varianceLinear3;

	m_meanBufferHybrid.push_back(mean);
	m_varianceBufferHybrid.push_back(variance);

	//////////////////////////////////////////////////////////////////////////////////////

	x = a_a[0];
	z = a_a[2];

	mean = m_linear->getMean({ x, z });
	variance = m_linear->getVariance({ x, z });


	getHeatMapColor(mean + 0.5, &r, &g, &b);

	m_colorBufferLinear_mean.push_back(r);
	m_colorBufferLinear_mean.push_back(g);
	m_colorBufferLinear_mean.push_back(b);

	m_meanBufferLinear.push_back(mean);

	getHeatMapColor(variance, &r, &g, &b);

	m_colorBufferLinear_variance.push_back(r);
	m_colorBufferLinear_variance.push_back(g);
	m_colorBufferLinear_variance.push_back(b);

	m_varianceBufferLinear.push_back(variance);

	x = a_b[0];
	z = a_b[2];

	mean = m_linear->getMean({ x, z });
	variance = m_linear->getVariance({ x, z });

	getHeatMapColor(mean + 0.5, &r, &g, &b);

	m_colorBufferLinear_mean.push_back(r);
	m_colorBufferLinear_mean.push_back(g);
	m_colorBufferLinear_mean.push_back(b);

	m_meanBufferLinear.push_back(mean);

	getHeatMapColor(variance, &r, &g, &b);

	m_colorBufferLinear_variance.push_back(r);
	m_colorBufferLinear_variance.push_back(g);
	m_colorBufferLinear_variance.push_back(b);

	m_varianceBufferLinear.push_back(variance);

	x = a_c[0];
	z = a_c[2];

	mean = m_linear->getMean({ x, z });
	variance = m_linear->getVariance({ x, z });

	getHeatMapColor(mean + 0.5, &r, &g, &b);

	m_colorBufferLinear_mean.push_back(r);
	m_colorBufferLinear_mean.push_back(g);
	m_colorBufferLinear_mean.push_back(b);

	m_meanBufferLinear.push_back(mean);

	getHeatMapColor(variance, &r, &g, &b);

	m_colorBufferLinear_variance.push_back(r);
	m_colorBufferLinear_variance.push_back(g);
	m_colorBufferLinear_variance.push_back(b);

	m_varianceBufferLinear.push_back(variance);

}

void TriangleGrid::clearBuffer(){

	m_vertexBuffer.clear();
	
	m_meanBufferHybrid.clear();
	m_meanBufferGPR.clear();

	m_varianceBufferHybrid.clear();
	m_varianceBufferGPR.clear();

	m_colorBufferLinear_mean.clear();
	m_colorBufferLinear_variance.clear();

	m_varianceBufferLinear.clear();
	m_varianceBufferLinear.clear();
}



void Triangle::setClockwise(){

	Vector3f tmp =Vector3f::cross(m_b - m_a, m_c - m_a);
	
	if (tmp[1] < 0){
		
		Vector3f vector = m_c;
		m_c = m_b;
		m_b = vector;

		double value = m_meanGPR3;
		m_meanGPR3 = m_meanGPR2;
		m_meanGPR2 = value;

		value = m_varianceGPR3;
		m_varianceGPR3 = m_varianceGPR2;
		m_varianceGPR2 = value;

		value = m_meanLinear3;
		m_meanLinear3 = m_meanLinear2;
		m_meanLinear2 = value;

		value = m_varianceLinear3;
		m_varianceLinear3 = m_varianceLinear2;
		m_varianceLinear2 = value;

	}
}