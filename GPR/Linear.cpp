#include "Linear.h"
#include "TriangleGrid.h"



Linear::Linear(){

}

Linear::Linear(std::vector<Triangle*> a_triangles){

	m_triangles = a_triangles;
}

Linear::~Linear(){


}

void Linear::hit(Vector3f pos){
	
	double epsilon = FLT_MAX;

	for (int i = 0; i < m_triangles.size(); i++){
	
		double A  = Triangle::area(m_triangles[i]->m_a, m_triangles[i]->m_b, m_triangles[i]->m_c);
		double A1 = Triangle::area(pos, m_triangles[i]->m_b, m_triangles[i]->m_c);
		double A2 = Triangle::area(m_triangles[i]->m_a, pos, m_triangles[i]->m_c);
		double A3 = Triangle::area(m_triangles[i]->m_a, m_triangles[i]->m_b, pos);
	
		double tmp = fabs(A - (A1 + A2 + A3));

		

		if (tmp == 0){
			m_triangle = m_triangles[i];
			m_A = A;
			m_A1 = A1;
			m_A2 = A2;
			m_A3 = A3;
			
			return;
		}else if (tmp < epsilon){
			epsilon = tmp;
			m_triangle = m_triangles[i];
			m_A = A;
			m_A1 = A1;
			m_A2 = A2;
			m_A3 = A3;

		}

	}
}




double Linear::getMean(std::vector<double>  s){

	hit(Vector3f(s[0], 0.0, s[1]));

	double alpha = m_A1 / m_A;
	double beta = m_A2 / m_A;
	double gamma = m_A3 / m_A;

	return alpha* m_triangle->m_meanGPR1 + beta * m_triangle->m_meanGPR2 + gamma * m_triangle->m_meanGPR3;
	
}

double Linear::getVariance(std::vector<double>  s){

	hit(Vector3f(s[0], 0.0, s[1]));

	double alpha = m_A1 / m_A;
	double beta = m_A2 / m_A;
	double gamma = m_A3 / m_A;

	return alpha * alpha * m_triangle->m_varianceGPR1 + beta * beta * m_triangle->m_varianceGPR2 + gamma * gamma * m_triangle->m_varianceGPR3;
}








