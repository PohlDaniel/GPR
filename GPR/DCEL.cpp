#include <string>
#include <array>
#include <fstream>
#include <functional>
#include <random>
#include <set>

#include "DCEL.h"

bool ComparePtr::operator()(DCEL_Face const* lhs, DCEL_Face const* rhs){

	return lhs->m_triangle->bhaError < rhs->m_triangle->bhaError;
}

bool compareVertex(DCEL_Vertex* const& lhs, DCEL_Vertex const& rhs){

	return lhs->m_position == rhs.m_position;
}


int DCEL_Vertex::found( DCEL& dcel){
	
	int index = -1;

	auto it = std::find_if(dcel.m_vertices.begin(), dcel.m_vertices.end(), std::bind(compareVertex, std::placeholders::_1, *this));
	if (it != dcel.m_vertices.end()){

		index = std::distance(dcel.m_vertices.begin(), it);

	}

	return index;
}


int DCEL_HalfEdge::found(DCEL& dcel){
	int index = -1;

	auto it = std::find_if(dcel.m_halfEdges.begin(), dcel.m_halfEdges.end(), std::bind(compareHalfEdge, std::placeholders::_1, *this));

	if (it != dcel.m_halfEdges.end()){

		index = std::distance(dcel.m_halfEdges.begin(), it);

	}

	return index;
}


void DCEL_HalfEdge::calcSplitpoint(DCEL_Face* face){

	if (m_splitpoint.calculated) return;

	double mu_p;
	double mu_q;
	double sigma_p;
	double sigma_q;
	double error1 = 0, error2 = 0, error3 = 0;
	double ab0, ab2, bc0, bc2, ca0, ca2;

	Vector3f a = m_dcel->m_vertices[m_origin]->m_position;
	Vector3f b = m_dcel->m_vertices[m_destination]->m_position;
	Vector3f ab;

	for (int j = 0; j < 100; j++) {

		double x = j / 100.0;
	
		ab = a + (b - a) * 0.5;
		mu_p = face->m_triangle->m_gpr->getMean({ ab[0], ab[2] });
		mu_q = face->m_triangle->getMean({ ab[0], ab[2] });
		sigma_p = face->m_triangle->m_gpr->getVariance({ ab[0], ab[2] });
		sigma_q = face->m_triangle->getVariance({ ab[0], ab[2] });

		double bha = 0.25*log(0.25*(sigma_p / sigma_q + sigma_q / sigma_p + 2)) + 0.25*((mu_p - mu_q)*(mu_p - mu_q) / (sigma_p + sigma_q));
		double mean = abs(mu_p - mu_q);

		if (max(mean, error1) > error1){

			error1 = max(mean, error1);
			ab0 = ab[0];
			ab2 = ab[2];
		}
	}
	
	m_splitpoint.calculated = true;

	float epsilon = 0.0000001;

	m_splitpoint.pos = Vector3f(ab0, 0.0, ab2);

	if ((fabs(ab0 - a[0]) < epsilon && fabs(ab2 - a[2]) < epsilon)){
		face->m_triangle->subdivide = false;
		m_splitpoint.calculated = false;
		
		m_splitpoint.index = m_dcel->m_vertices[m_origin]->m_id;

		return;
	}

	if ((fabs(ab0 - b[0]) < epsilon && fabs(ab2 - b[2]) < epsilon)){
		face->m_triangle->subdivide = false;
		m_splitpoint.calculated = false;

		m_splitpoint.index = m_dcel->m_vertices[m_destination]->m_id;

		return;
	}

	DCEL_Vertex* splitpoint = new DCEL_Vertex(m_splitpoint.pos, m_dcel);
	splitpoint->add(false);
	m_splitpoint.index = splitpoint->m_id;

}

void DCEL_HalfEdge::getChilds(std::vector<int> &a_childs){

	if (m_child1 == -1){
		
		a_childs.push_back(m_id);
		return;
		

	}else{
		a_childs.push_back(m_id);
		m_dcel->m_halfEdges[m_child1]->getChilds(a_childs);
		m_dcel->m_halfEdges[m_child2]->getChilds(a_childs);

	}

}

bool compareFace(DCEL_Face* const& lhs, DCEL_Face const& rhs){

	return lhs->m_triangle->m_a == rhs.m_triangle->m_a   && lhs->m_triangle->m_b == rhs.m_triangle->m_b && lhs->m_triangle->m_c == rhs.m_triangle->m_c;
}


int DCEL_Face::found(DCEL& dcel){
	int index = -1;

	auto it = std::find_if(dcel.m_faces.begin(), dcel.m_faces.end(), std::bind(compareFace, std::placeholders::_1, *this));

	if (it != dcel.m_faces.end()){

		index = std::distance(dcel.m_faces.begin(), it);

	}

	return index;
}

void DCEL_Face::showEdges(){

	int begin = m_edge;
	int prev = m_dcel->m_halfEdges[begin]->m_previous;
	int next = m_dcel->m_halfEdges[begin]->m_next;

	std::cout << "edge1: " << std::endl;
	std::cout << "pev |id | next: " << prev << " | " << begin << " | " << next << std::endl;
	std::cout << "origin | destination: " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[2] << " | " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[2] << std::endl;
	std::cout << "twin: " << m_dcel->m_halfEdges[begin]->m_twin << std::endl;
	std::cout << "face: " << m_dcel->m_halfEdges[begin]->m_face << std::endl;

	if (m_dcel->m_halfEdges[begin]->m_twin2 > -1){

		std::cout << "twin2: " << m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin2]->m_id << std::endl;
	}

	std::cout << "-----------------------------------------------" << std::endl;

    begin = m_dcel->m_halfEdges[begin]->m_next;
    prev = m_dcel->m_halfEdges[begin]->m_previous;prev = m_dcel->m_halfEdges[begin]->m_previous;
	next = m_dcel->m_halfEdges[begin]->m_next;

	std::cout << "edge2: " << std::endl;
	std::cout << "pev |id | next: " << prev << " | " << begin << " | " << next << std::endl;
	std::cout << "origin | destination: " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[2] << " | " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[2] << std::endl;
	std::cout << "twin: " << m_dcel->m_halfEdges[begin]->m_twin << std::endl;
	std::cout << "face: " << m_dcel->m_halfEdges[begin]->m_face << std::endl;

	if (m_dcel->m_halfEdges[begin]->m_twin2 > -1){

		std::cout << "twin2: " << m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin2]->m_id << std::endl;
	}

	std::cout << "-----------------------------------------------" << std::endl;

	begin = m_dcel->m_halfEdges[begin]->m_next;
	prev = m_dcel->m_halfEdges[begin]->m_previous; prev = m_dcel->m_halfEdges[begin]->m_previous;
	next = m_dcel->m_halfEdges[begin]->m_next;

	std::cout << "edge3: " << std::endl;
	std::cout << "pev |id | next: " << prev << " | " << begin << " | " << next << std::endl;
	std::cout << "origin | destination: " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position[2] << " | " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[0] << "  " << m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_destination]->m_position[2] << std::endl;
	std::cout << "twin: " << m_dcel->m_halfEdges[begin]->m_twin << std::endl;
	std::cout << "face: " << m_dcel->m_halfEdges[begin]->m_face << std::endl;

	if (m_dcel->m_halfEdges[begin]->m_twin2 > -1){

		std::cout << "twin2: " << m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin2]->m_id << std::endl;
	}

	std::cout << "##########################################################" << std::endl;
}


void DCEL_Face::split4(){
	
	int begin = m_edge;
	int next1 = m_dcel->m_halfEdges[begin]->m_next;
	int next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;

	
	DCEL_Vertex* _splitpoint1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_splitpoint.index];
	DCEL_Vertex* _splitpoint2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_splitpoint.index];
	DCEL_Vertex* _splitpoint3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_splitpoint.index];

	m_dcel->m_halfEdges[begin]->m_splitpoint.calculated = false;
	m_dcel->m_halfEdges[next1]->m_splitpoint.calculated = false;
	m_dcel->m_halfEdges[next2]->m_splitpoint.calculated = false;

	_splitpoint1->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
	_splitpoint1->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
	_splitpoint1->setMeanLinear(m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
	_splitpoint1->setVarianceLinear(m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));

	/*_splitpoint1->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	_splitpoint1->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	_splitpoint1->m_meanLinear = m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	_splitpoint1->m_varianceLinear = m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });*/
	
	_splitpoint2->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
	_splitpoint2->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
	_splitpoint2->setMeanLinear(m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
	_splitpoint2->setVarianceLinear(m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));

	/*_splitpoint2->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	_splitpoint2->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	_splitpoint2->m_meanLinear = m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	_splitpoint2->m_varianceLinear = m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });*/
	
	_splitpoint3->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
	_splitpoint3->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
	_splitpoint3->setMeanLinear(m_triangle->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
	_splitpoint3->setVarianceLinear(m_triangle->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));


	/*_splitpoint3->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
	_splitpoint3->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
	_splitpoint3->m_meanLinear = m_triangle->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
	_splitpoint3->m_varianceLinear = m_triangle->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });*/
	

	DCEL_Vertex* _a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin];
	DCEL_Vertex* _b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin];
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin];
	
	
	_a->m_meanGPR = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceGPR = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	//_a->m_meanLinear = m_triangle->getMean({ _a->m_position[0], _a->m_position[2] });
	//_a->m_varianceLinear = m_triangle->getVariance({ _a->m_position[0], _a->m_position[2] });
	_a->m_meanLinear = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceLinear = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	_b->m_meanGPR = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceGPR = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	//_b->m_meanLinear = m_triangle->getMean({ _b->m_position[0], _b->m_position[2] });
	//_b->m_varianceLinear = m_triangle->getVariance({ _b->m_position[0], _b->m_position[2] });
	_b->m_meanLinear = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceLinear = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	_c->m_meanGPR = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceGPR = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	//_c->m_meanLinear = m_triangle->getMean({ _c->m_position[0], _c->m_position[2] });
	//_c->m_varianceLinear = m_triangle->getVariance({ _c->m_position[0], _c->m_position[2] });
	_c->m_meanLinear = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceLinear = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });


	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];
	DCEL_HalfEdge* halfEdge6 = new DCEL_HalfEdge(m_dcel);
	halfEdge6->add(false);
	DCEL_HalfEdge* halfEdge7 = new DCEL_HalfEdge(m_dcel);
	halfEdge7->add(false);
	DCEL_HalfEdge* halfEdge8 = new DCEL_HalfEdge(m_dcel);
	halfEdge8->add(false);
	DCEL_HalfEdge* halfEdge9 = new DCEL_HalfEdge(m_dcel);
	halfEdge9->add(false);
	DCEL_HalfEdge* halfEdge10 = new DCEL_HalfEdge(m_dcel);
	halfEdge10->add(false);
	DCEL_HalfEdge* halfEdge11 = new DCEL_HalfEdge(m_dcel);
	halfEdge11->add(false);
	DCEL_HalfEdge* halfEdge12 = new DCEL_HalfEdge(m_dcel);
	halfEdge12->add(false);

	DCEL_HalfEdge *halfEdge13 = new DCEL_HalfEdge(m_dcel);
	halfEdge13->add(false);
	halfEdge13->m_origin = _a->m_id;
	halfEdge13->m_destination = _splitpoint1->m_id;


	DCEL_HalfEdge *halfEdge14 = new DCEL_HalfEdge(m_dcel);
	halfEdge14->add(false);
	halfEdge14->m_origin = _splitpoint1->m_id;
	halfEdge14->m_destination = _splitpoint3->m_id;

	halfEdge13->m_next = halfEdge14->m_id;
	halfEdge13->m_previous = halfEdge2->m_id;
	halfEdge13->m_face = m_id;

	halfEdge14->m_next = halfEdge2->m_id;
	halfEdge14->m_previous = halfEdge13->m_id;
	halfEdge14->m_face = m_id;

	halfEdge2->m_origin = _splitpoint3->m_id;
	halfEdge2->m_next = halfEdge13->m_id;
	halfEdge2->m_previous = halfEdge14->m_id;

	Triangle* triangle2 = new Triangle(_splitpoint1->m_position, _b->m_position, _splitpoint2->m_position,
		_splitpoint1->m_meanGPR,
		_b->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_splitpoint1->m_varianceGPR,
		_b->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);
	
	
	DCEL_Face* face2 = new DCEL_Face(triangle2, m_dcel);
	face2->addAll(_splitpoint1, _b, _splitpoint2, halfEdge0, halfEdge6, halfEdge7, false);
	

	Triangle* triangle3 = new Triangle(_splitpoint1->m_position, _splitpoint2->m_position, _splitpoint3->m_position,
		_splitpoint1->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_splitpoint3->m_meanGPR,
		_splitpoint1->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		_splitpoint3->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face3 = new DCEL_Face(triangle3, m_dcel);
	face3->addAll(_splitpoint1, _splitpoint2, _splitpoint3, halfEdge8, halfEdge9, halfEdge10, false);
	
	Triangle* triangle4 = new Triangle(_splitpoint3->m_position, _splitpoint2->m_position, _c->m_position,
		_splitpoint3->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_c->m_meanGPR,
		_splitpoint3->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		_c->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);


	DCEL_Face* face4 = new DCEL_Face(triangle4, m_dcel);
	face4->addAll(_splitpoint3, _splitpoint2, _c, halfEdge11, halfEdge1, halfEdge12, false);
	
	// set inner twin Face1 
	halfEdge13->m_twin = halfEdge0->m_twin;
	halfEdge14->m_twin = halfEdge10->m_id;

	// set twins Face2
	halfEdge6->m_twin = halfEdge1->m_twin;
	halfEdge7->m_twin = halfEdge8->m_id;
	
		
	// set twins Face3
	halfEdge8->m_twin = halfEdge7->m_id;
	halfEdge9->m_twin = halfEdge11->m_id;
	halfEdge10->m_twin = halfEdge14->m_id;

	// set twins Face4
	halfEdge11->m_twin = halfEdge9->m_id;
	halfEdge12->m_twin = halfEdge2->m_twin;

	
	// set double twins Face2
	if (halfEdge0->m_twin2 > -1){

		halfEdge0->m_twin = halfEdge0->m_twin2;
		m_dcel->m_halfEdges[halfEdge13->m_twin]->m_twin = halfEdge13->m_id;
		halfEdge0->m_twin2 = -1;

	}
	else if (!m_dcel->m_halfEdges[halfEdge0->m_twin]->m_border){
		halfEdge13->m_twin = halfEdge0->m_twin;
		m_dcel->m_halfEdges[halfEdge0->m_twin]->m_twin2 = halfEdge13->m_id;
	}

	// set double twins Face4
	if (halfEdge1->m_twin2 > -1){

		halfEdge1->m_twin = halfEdge1->m_twin2;
		m_dcel->m_halfEdges[halfEdge6->m_twin]->m_twin = halfEdge6->m_id;

		halfEdge1->m_twin2 = -1;

	}
	else if (!m_dcel->m_halfEdges[halfEdge1->m_twin]->m_border){

		halfEdge6->m_twin = halfEdge1->m_twin;

		m_dcel->m_halfEdges[halfEdge1->m_twin]->m_twin2 = halfEdge6->m_id;
	}

	// set double twins Face1
	if (halfEdge2->m_twin2 > -1){

		halfEdge2->m_twin = halfEdge2->m_twin2;
		m_dcel->m_halfEdges[halfEdge12->m_twin]->m_twin = halfEdge12->m_id;
		halfEdge2->m_twin2 = -1;

	}
	else if (!m_dcel->m_halfEdges[halfEdge2->m_twin]->m_border){

		halfEdge12->m_twin = halfEdge2->m_twin;
		m_dcel->m_halfEdges[halfEdge2->m_twin]->m_twin2 = halfEdge12->m_id;
	}

	// set face 1
	m_triangle->m_meanGPR1 = _a->m_meanGPR;
	m_triangle->m_meanGPR2 = _splitpoint1->m_meanGPR;
	m_triangle->m_meanGPR3 = _splitpoint3->m_meanGPR;
	m_triangle->m_varianceGPR1 = _a->m_varianceGPR;
	m_triangle->m_varianceGPR2 = _splitpoint1->m_varianceGPR;
	m_triangle->m_varianceGPR3 = _splitpoint3->m_varianceGPR;
	m_triangle->m_b = _splitpoint1->m_position;
	m_triangle->m_c = _splitpoint3->m_position;
	m_triangle->errorCalaculated = false;
	m_edge = halfEdge13->m_id;
}


void DCEL_Face::split6(){
	
	int begin = m_edge;
	int next1 = m_dcel->m_halfEdges[begin]->m_next;
	int next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;

	

	m_dcel->m_halfEdges[begin]->m_splitpoint.calculated = false;
	m_dcel->m_halfEdges[next1]->m_splitpoint.calculated = false;
	m_dcel->m_halfEdges[next2]->m_splitpoint.calculated = false;

	DCEL_Vertex* _splitpoint1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_splitpoint.index];
	DCEL_Vertex* _splitpoint2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_splitpoint.index];
	DCEL_Vertex* _splitpoint3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_splitpoint.index];


	if (_splitpoint1->m_meanGPR == FLT_MAX){

		_splitpoint1->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_meanLinear = m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_varianceLinear = m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	}

	if (_splitpoint2->m_meanGPR == FLT_MAX){

		_splitpoint2->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_meanLinear = m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_varianceLinear = m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	}

	if (_splitpoint3->m_meanGPR == FLT_MAX){

		_splitpoint3->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
		_splitpoint3->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
		_splitpoint3->m_meanLinear = m_triangle->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
		_splitpoint3->m_varianceLinear = m_triangle->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] });
	}

	DCEL_Vertex* _a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin];
	DCEL_Vertex* _b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin];
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin];

	/*_a->m_meanGPR = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceGPR = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	//_a->m_meanLinear = m_triangle->getMean({ _a->m_position[0], _a->m_position[2] });
	//_a->m_varianceLinear = m_triangle->getVariance({ _a->m_position[0], _a->m_position[2] });
	_a->m_meanLinear = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceLinear = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	_b->m_meanGPR = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceGPR = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	//_b->m_meanLinear = m_triangle->getMean({ _b->m_position[0], _b->m_position[2] });
	//_b->m_varianceLinear = m_triangle->getVariance({ _b->m_position[0], _b->m_position[2] });
	_b->m_meanLinear = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceLinear = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	_c->m_meanGPR = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceGPR = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	//_c->m_meanLinear = m_triangle->getMean({ _c->m_position[0], _c->m_position[2] });
	//_c->m_varianceLinear = m_triangle->getVariance({ _c->m_position[0], _c->m_position[2] });
	_c->m_meanLinear = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceLinear = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });*/

	Vector3f midpoint = (_a->m_position + _b->m_position + _c->m_position) / 3.0;
	DCEL_Vertex* _midpoint = new DCEL_Vertex(midpoint, m_dcel);
	_midpoint->add(false);


	_midpoint->m_meanGPR = m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_varianceGPR = m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_meanLinear = m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_varianceLinear = m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });



	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];
	DCEL_HalfEdge* halfEdge6 = new DCEL_HalfEdge(m_dcel);
	halfEdge6->add(false);
	DCEL_HalfEdge* halfEdge7 = new DCEL_HalfEdge(m_dcel);
	halfEdge7->add(false);
	DCEL_HalfEdge* halfEdge8 = new DCEL_HalfEdge(m_dcel);
	halfEdge8->add(false);
	DCEL_HalfEdge* halfEdge9 = new DCEL_HalfEdge(m_dcel);
	halfEdge9->add(false);
	DCEL_HalfEdge* halfEdge10 = new DCEL_HalfEdge(m_dcel);
	halfEdge10->add(false);
	DCEL_HalfEdge* halfEdge11 = new DCEL_HalfEdge(m_dcel);
	halfEdge11->add(false);
	DCEL_HalfEdge* halfEdge12 = new DCEL_HalfEdge(m_dcel);
	halfEdge12->add(false);
	DCEL_HalfEdge* halfEdge13 = new DCEL_HalfEdge(m_dcel);
	halfEdge13->add(false);
	DCEL_HalfEdge* halfEdge14 = new DCEL_HalfEdge(m_dcel);
	halfEdge14->add(false);
	DCEL_HalfEdge* halfEdge15 = new DCEL_HalfEdge(m_dcel);
	halfEdge15->add(false);
	DCEL_HalfEdge* halfEdge16 = new DCEL_HalfEdge(m_dcel);
	halfEdge16->add(false);
	DCEL_HalfEdge* halfEdge17 = new DCEL_HalfEdge(m_dcel);
	halfEdge17->add(false);

	DCEL_HalfEdge *halfEdge18 = new DCEL_HalfEdge(m_dcel);
	halfEdge18->add(false);
	halfEdge18->m_origin = _a->m_id;
	halfEdge18->m_destination = _splitpoint1->m_id;


	DCEL_HalfEdge *halfEdge19 = new DCEL_HalfEdge(m_dcel);
	halfEdge19->add(false);
	halfEdge19->m_origin = _splitpoint1->m_id;
	halfEdge19->m_destination = _midpoint->m_id;

	DCEL_HalfEdge *halfEdge20 = new DCEL_HalfEdge(m_dcel);
	halfEdge20->add(false);
	halfEdge20->m_origin = _midpoint->m_id;
	halfEdge20->m_destination = _a->m_id;

	halfEdge18->m_next = halfEdge19->m_id;
	halfEdge18->m_previous = halfEdge20->m_id;
	halfEdge18->m_face = m_id;

	halfEdge19->m_next = halfEdge20->m_id;
	halfEdge19->m_previous = halfEdge18->m_id;
	halfEdge19->m_face = m_id;

	halfEdge20->m_next = halfEdge18->m_id;
	halfEdge20->m_previous = halfEdge19->m_id;
	halfEdge20->m_face = m_id;

	Triangle* triangle2 = new Triangle(_splitpoint1->m_position, _b->m_position, _midpoint->m_position,
		m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }),
		m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face2 = new DCEL_Face(triangle2, m_dcel);
	face2->addAll(_splitpoint1, _b, _midpoint, halfEdge0, halfEdge6, halfEdge7, false);

	Triangle* triangle3 = new Triangle(_b->m_position, _splitpoint2->m_position, _midpoint->m_position,
		m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face3 = new DCEL_Face(triangle3, m_dcel);
	face3->addAll(_b, _splitpoint2, _midpoint, halfEdge8, halfEdge9, halfEdge10, false);
	
	Triangle* triangle4 = new Triangle(_splitpoint2->m_position, _c->m_position, _midpoint->m_position,
		m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face4 = new DCEL_Face(triangle4, m_dcel);
	face4->addAll(_splitpoint2, _c, _midpoint, halfEdge1, halfEdge11, halfEdge12, false);

	Triangle* triangle5 = new Triangle(_c->m_position, _splitpoint3->m_position, _midpoint->m_position,
		m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }),
		m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face5 = new DCEL_Face(triangle5, m_dcel);
	face5->addAll(_c, _splitpoint3, _midpoint, halfEdge13, halfEdge14, halfEdge15, false);

	Triangle* triangle6 = new Triangle(_splitpoint3->m_position, _a->m_position, _midpoint->m_position,
		m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }),
		m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face6 = new DCEL_Face(triangle6, m_dcel);
	face6->addAll(_splitpoint3, _a, _midpoint, halfEdge2, halfEdge16, halfEdge17, false);


	// set twins Face1
	halfEdge18->m_twin = halfEdge0->m_twin;
	halfEdge19->m_twin = halfEdge7->m_id;
	halfEdge20->m_twin = halfEdge16->m_id;

	// set twins Face2
	halfEdge6->m_twin = halfEdge10->m_id;
	halfEdge7->m_twin = halfEdge19->m_id;

	// set twins Face3
	halfEdge8->m_twin = halfEdge1->m_twin;
	halfEdge9->m_twin = halfEdge12->m_id;
	halfEdge10->m_twin = halfEdge6->m_id;

	// set twins Face4
	halfEdge11->m_twin = halfEdge15->m_id;
	halfEdge12->m_twin = halfEdge9->m_id;

	// set twins Face5
	halfEdge13->m_twin = halfEdge2->m_twin;
	halfEdge14->m_twin = halfEdge17->m_id;
	halfEdge15->m_twin = halfEdge11->m_id;

	// set twins Face6
	halfEdge16->m_twin = halfEdge20->m_id;
	halfEdge17->m_twin = halfEdge14->m_id;


	// set double twins Face2
	if (halfEdge0->m_twin2 > -1){

		halfEdge0->m_twin = halfEdge0->m_twin2;
		m_dcel->m_halfEdges[halfEdge18->m_twin]->m_twin = halfEdge18->m_id;
		halfEdge0->m_twin2 = -1;

	}else if (!m_dcel->m_halfEdges[halfEdge0->m_twin]->m_border){

		halfEdge18->m_twin = halfEdge0->m_twin;
		m_dcel->m_halfEdges[halfEdge0->m_twin]->m_twin2 = halfEdge18->m_id;
	}

	// set double twins Face4
	if (halfEdge1->m_twin2 > -1){

		halfEdge1->m_twin = halfEdge1->m_twin2;
		m_dcel->m_halfEdges[halfEdge8->m_twin]->m_twin = halfEdge8->m_id;
		halfEdge1->m_twin2 = -1;

	}else if (!m_dcel->m_halfEdges[halfEdge1->m_twin]->m_border){

		halfEdge8->m_twin = halfEdge1->m_twin;
		m_dcel->m_halfEdges[halfEdge1->m_twin]->m_twin2 = halfEdge8->m_id;
	}

	// set double twins Face6
	if (halfEdge2->m_twin2 > -1){

		halfEdge2->m_twin = halfEdge2->m_twin2;
		m_dcel->m_halfEdges[halfEdge13->m_twin]->m_twin = halfEdge13->m_id;
		halfEdge2->m_twin2 = -1;

	}
	else if (!m_dcel->m_halfEdges[halfEdge2->m_twin]->m_border){

		halfEdge13->m_twin = halfEdge2->m_twin;
		m_dcel->m_halfEdges[halfEdge2->m_twin]->m_twin2 = halfEdge13->m_id;
	}

	// set face 1
	m_triangle->m_meanGPR1 = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_meanGPR2 = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	m_triangle->m_meanGPR3 = m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	m_triangle->m_varianceGPR1 = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_varianceGPR2 = m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	m_triangle->m_varianceGPR3 = m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });
	m_triangle->m_b = _splitpoint1->m_position;
	m_triangle->m_c = _midpoint->m_position;
	m_triangle->errorCalaculated = false;
	m_edge = halfEdge18->m_id;
}

void DCEL_Face::split2(int index){
	
	int begin = m_dcel->m_halfEdges[index]->m_previous;
	int next1 = m_dcel->m_halfEdges[index]->m_id;
	int next2 = m_dcel->m_halfEdges[index]->m_next;

	
	m_dcel->m_halfEdges[next1]->m_splitpoint.calculated = false;

	DCEL_Vertex* _splitpoint = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_splitpoint.index];

	if (_splitpoint->m_meanGPR == FLT_MAX){

		_splitpoint->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
		_splitpoint->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
		_splitpoint->m_meanLinear = m_triangle->getMean({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
		_splitpoint->m_varianceLinear = m_triangle->getVariance({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
	}

	DCEL_Vertex* _a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin];
	DCEL_Vertex* _b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin];
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin];

	/*_a->m_meanGPR = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceGPR = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	//_a->m_meanLinear = m_triangle->getMean({ _a->m_position[0], _a->m_position[2] });
	//_a->m_varianceLinear = m_triangle->getVariance({ _a->m_position[0], _a->m_position[2] });
	_a->m_meanLinear = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceLinear = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	_b->m_meanGPR = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceGPR = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	//_b->m_meanLinear = m_triangle->getMean({ _b->m_position[0], _b->m_position[2] });
	//_b->m_varianceLinear = m_triangle->getVariance({ _b->m_position[0], _b->m_position[2] });
	_b->m_meanLinear = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceLinear = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	_c->m_meanGPR = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceGPR = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	//_c->m_meanLinear = m_triangle->getMean({ _c->m_position[0], _c->m_position[2] });
	//_c->m_varianceLinear = m_triangle->getVariance({ _c->m_position[0], _c->m_position[2] });
	_c->m_meanLinear = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceLinear = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });*/

	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];
	DCEL_HalfEdge* halfEdge6 = new DCEL_HalfEdge(m_dcel);
	halfEdge6->add(false);
	DCEL_HalfEdge* halfEdge7 = new DCEL_HalfEdge(m_dcel);
	halfEdge7->add(false);
	DCEL_HalfEdge* halfEdge8 = new DCEL_HalfEdge(m_dcel);
	halfEdge8->add(false);
	

	halfEdge0->m_origin = _a->m_id;
	halfEdge0->m_destination = _b->m_id;

	halfEdge6->m_origin = _b->m_id;
	halfEdge6->m_destination = _splitpoint->m_id;

	halfEdge7->m_origin = _splitpoint->m_id;
	halfEdge7->m_destination = _a->m_id;

	halfEdge0->m_next = halfEdge6->m_id;
	halfEdge0->m_previous = halfEdge7->m_id;
	halfEdge0->m_face = m_id;

	halfEdge6->m_next = halfEdge7->m_id;
	halfEdge6->m_previous = halfEdge0->m_id;
	halfEdge6->m_face = m_id;

	halfEdge7->m_next = halfEdge0->m_id;
	halfEdge7->m_previous = halfEdge6->m_id;
	halfEdge7->m_face = m_id;

	Triangle* triangle2 = new Triangle(_a->m_position, _splitpoint->m_position, _c->m_position,
		m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getMean({ _splitpoint->m_position[0], _splitpoint->m_position[2] }),
		m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint->m_position[0], _splitpoint->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);


	DCEL_Face* face2 = new DCEL_Face(triangle2, m_dcel);
	face2->addAll(_a, _splitpoint, _c, halfEdge8, halfEdge1, halfEdge2, false);

	// set twins Face1
	halfEdge0->m_twin = halfEdge0->m_twin;
	halfEdge6->m_twin = halfEdge1->m_twin;
	halfEdge7->m_twin = halfEdge8->m_id;

	// set twins Face2
	halfEdge2->m_twin = halfEdge2->m_twin;
	halfEdge1->m_twin = halfEdge1->m_twin;
	halfEdge8->m_twin = halfEdge7->m_id;

	// set double twins Face2
	if (halfEdge1->m_twin2 > -1){

		halfEdge1->m_twin = halfEdge1->m_twin2;
		m_dcel->m_halfEdges[halfEdge6->m_twin]->m_twin = halfEdge6->m_id;
		halfEdge1->m_twin2 = -1;

	}else if (!m_dcel->m_halfEdges[halfEdge1->m_twin]->m_border){

		halfEdge6->m_twin = halfEdge1->m_twin;
		m_dcel->m_halfEdges[halfEdge1->m_twin]->m_twin2 = halfEdge6->m_id;
	}

	// set face 1
	m_triangle->m_meanGPR1 = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_meanGPR2 = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	m_triangle->m_meanGPR3 = m_triangle->m_gpr->getMean({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
	m_triangle->m_varianceGPR1 = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_varianceGPR2 = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });
	m_triangle->m_varianceGPR3 = m_triangle->m_gpr->getVariance({ _splitpoint->m_position[0], _splitpoint->m_position[2] });
	m_edge = halfEdge0->m_id;
	m_triangle->m_a = _a->m_position;
	m_triangle->m_b = _b->m_position;
	m_triangle->m_c = _splitpoint->m_position;
	m_triangle->errorCalaculated = false;
}

void DCEL_Face::split3(int index1, int index2){
	
	int begin = index1;
	int next1 = m_dcel->m_halfEdges[index2]->m_id;
	int next2 = m_dcel->m_halfEdges[index2]->m_next;

	

	m_dcel->m_halfEdges[begin]->m_splitpoint.calculated = false;
	m_dcel->m_halfEdges[next1]->m_splitpoint.calculated = false;
	

	DCEL_Vertex* _splitpoint1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_splitpoint.index];
	DCEL_Vertex* _splitpoint2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_splitpoint.index];

	if (_splitpoint1->m_meanGPR == FLT_MAX){

		_splitpoint1->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_meanLinear = m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
		_splitpoint1->m_varianceLinear = m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	}

	if (_splitpoint2->m_meanGPR == FLT_MAX){

		_splitpoint2->m_meanGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_varianceGPR = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_meanLinear = m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
		_splitpoint2->m_varianceLinear = m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	}

	DCEL_Vertex* _a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin];
	DCEL_Vertex* _b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin];
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin];

	/*_a->m_meanGPR = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceGPR = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	//_a->m_meanLinear = m_triangle->getMean({ _a->m_position[0], _a->m_position[2] });
	//_a->m_varianceLinear = m_triangle->getVariance({ _a->m_position[0], _a->m_position[2] });
	_a->m_meanLinear = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceLinear = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	_b->m_meanGPR = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceGPR = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	//_b->m_meanLinear = m_triangle->getMean({ _b->m_position[0], _b->m_position[2] });
	//_b->m_varianceLinear = m_triangle->getVariance({ _b->m_position[0], _b->m_position[2] });
	_b->m_meanLinear = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceLinear = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	_c->m_meanGPR = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceGPR = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	//_c->m_meanLinear = m_triangle->getMean({ _c->m_position[0], _c->m_position[2] });
	//_c->m_varianceLinear = m_triangle->getVariance({ _c->m_position[0], _c->m_position[2] });
	_c->m_meanLinear = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceLinear = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });*/


	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];
	DCEL_HalfEdge* halfEdge6 = new DCEL_HalfEdge(m_dcel);
	halfEdge6->add(false);
	DCEL_HalfEdge* halfEdge7 = new DCEL_HalfEdge(m_dcel);
	halfEdge7->add(false);
	DCEL_HalfEdge* halfEdge8 = new DCEL_HalfEdge(m_dcel);
	halfEdge8->add(false);
	
	DCEL_HalfEdge *halfEdge9 = new DCEL_HalfEdge(m_dcel);
	halfEdge9->add(false);
	halfEdge9->m_origin = _a->m_id;
	halfEdge9->m_destination = _splitpoint1->m_id;


	DCEL_HalfEdge *halfEdge10 = new DCEL_HalfEdge(m_dcel);
	halfEdge10->add(false);
	halfEdge10->m_origin = _splitpoint1->m_id;
	halfEdge10->m_destination = _splitpoint2->m_id;

	DCEL_HalfEdge *halfEdge11 = new DCEL_HalfEdge(m_dcel);
	halfEdge11->add(false);
	halfEdge11->m_origin = _splitpoint2->m_id;
	halfEdge11->m_destination = _a->m_id;

	halfEdge9->m_next = halfEdge10->m_id;
	halfEdge9->m_previous = halfEdge11->m_id;
	halfEdge9->m_face = m_id;

	halfEdge10->m_next = halfEdge11->m_id;
	halfEdge10->m_previous = halfEdge9->m_id;
	halfEdge10->m_face = m_id;

	halfEdge11->m_next = halfEdge9->m_id;
	halfEdge11->m_previous = halfEdge10->m_id;
	halfEdge11->m_face = m_id;

	Triangle* triangle2 = new Triangle(_splitpoint1->m_position, _b->m_position, _splitpoint2->m_position,
		m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }),
		m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face2 = new DCEL_Face(triangle2, m_dcel);
	face2->addAll(_splitpoint1, _b, _splitpoint2, halfEdge0, halfEdge6, halfEdge7, false);

	Triangle* triangle3 = new Triangle(_a->m_position, _splitpoint2->m_position, _c->m_position,
		m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }),
		m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] }),
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face3 = new DCEL_Face(triangle3, m_dcel);
	face3->addAll(_a, _splitpoint2, _c, halfEdge8, halfEdge1, halfEdge2, false);


	// set twins Face1
	halfEdge9->m_twin = halfEdge0->m_twin;
	halfEdge10->m_twin = halfEdge7->m_id;
	halfEdge11->m_twin = halfEdge8->m_id;

	// set twins Face2
	halfEdge0->m_twin = halfEdge0->m_twin;
	halfEdge7->m_twin = halfEdge10->m_id;
	halfEdge6->m_twin = halfEdge1->m_twin;

	// set twins Face3
	halfEdge8->m_twin = halfEdge11->m_id;
	halfEdge1->m_twin = halfEdge1->m_twin;
	halfEdge2->m_twin = halfEdge2->m_twin;


	// set double twins Face2
	if (halfEdge0->m_twin2 > -1){

		halfEdge0->m_twin = halfEdge0->m_twin2;
		m_dcel->m_halfEdges[halfEdge9->m_twin]->m_twin = halfEdge9->m_id;
		halfEdge0->m_twin2 = -1;

	}else if (!m_dcel->m_halfEdges[halfEdge0->m_twin]->m_border){

		halfEdge9->m_twin = halfEdge0->m_twin;
		m_dcel->m_halfEdges[halfEdge0->m_twin]->m_twin2 = halfEdge9->m_id;
	}

	// set double twins Face4
	if (halfEdge1->m_twin2 > -1){

		halfEdge1->m_twin = halfEdge1->m_twin2;
		m_dcel->m_halfEdges[halfEdge6->m_twin]->m_twin = halfEdge6->m_id;
		halfEdge1->m_twin2 = -1;

	}else if (!m_dcel->m_halfEdges[halfEdge1->m_twin]->m_border){

		halfEdge6->m_twin = halfEdge1->m_twin;
		m_dcel->m_halfEdges[halfEdge1->m_twin]->m_twin2 = halfEdge6->m_id;
	}

	// set face 1
	m_triangle->m_meanGPR1 = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_meanGPR2 = m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	m_triangle->m_meanGPR3 = m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	m_triangle->m_varianceGPR1 = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });
	m_triangle->m_varianceGPR2 = m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] });
	m_triangle->m_varianceGPR3 = m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] });
	m_triangle->m_a = _a->m_position;
	m_triangle->m_b = _splitpoint1->m_position;
	m_triangle->m_c = _splitpoint2->m_position;
	m_triangle->errorCalaculated = false;
	m_edge = halfEdge9->m_id;
}

void  DCEL_Face::_split4(){

	int begin = m_edge;
	int next1 = m_dcel->m_halfEdges[begin]->m_next;
	int next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;

	DCEL_Vertex* _splitpoint1;
	DCEL_Vertex* _splitpoint2;
	DCEL_Vertex* _splitpoint3;

	if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin]->m_child1 > -1 && m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin]->m_splitpoint.calculated){

		_splitpoint1 = m_dcel->m_vertices[m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin]->m_splitpoint.index];

		_splitpoint1->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
		_splitpoint1->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
		_splitpoint1->setMeanLinear(m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
		_splitpoint1->setVarianceLinear(m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));

	}else{

			m_dcel->m_halfEdges[begin]->calcSplitpoint( this);

			if (m_dcel->m_halfEdges[begin]->m_splitpoint.calculated){

				_splitpoint1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_splitpoint.index];

			}else{

				return;
			}

			_splitpoint1->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
			_splitpoint1->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
			_splitpoint1->setMeanLinear(m_triangle->getMean({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
			_splitpoint1->setVarianceLinear(m_triangle->getVariance({ _splitpoint1->m_position[0], _splitpoint1->m_position[2] }));
	}
 

	if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin]->m_child1 > -1 && m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin]->m_splitpoint.calculated){

		_splitpoint2 = m_dcel->m_vertices[m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin]->m_splitpoint.index];

		_splitpoint2->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setMeanLinear(m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setVarianceLinear(m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));

	}else{


		m_dcel->m_halfEdges[next1]->calcSplitpoint(this);

		if (m_dcel->m_halfEdges[next1]->m_splitpoint.calculated){

			_splitpoint2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_splitpoint.index];

		}else{

			return;
		}

		_splitpoint2->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setMeanLinear(m_triangle->getMean({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
		_splitpoint2->setVarianceLinear(m_triangle->getVariance({ _splitpoint2->m_position[0], _splitpoint2->m_position[2] }));
	}
	
	if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin]->m_child1 > -1 && m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin]->m_splitpoint.calculated){
		
		_splitpoint3 = m_dcel->m_vertices[m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin]->m_splitpoint.index];

		_splitpoint3->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setMeanLinear(m_triangle->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setVarianceLinear(m_triangle->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));

	}else{

		m_dcel->m_halfEdges[next2]->calcSplitpoint(this);

		if (m_dcel->m_halfEdges[next2]->m_splitpoint.calculated){

			_splitpoint3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_splitpoint.index];

		}else{

			return;
		}

		_splitpoint3->setMeanGPR(m_triangle->m_gpr->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setVarianceGPR(m_triangle->m_gpr->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setMeanLinear(m_triangle->getMean({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
		_splitpoint3->setVarianceLinear(m_triangle->getVariance({ _splitpoint3->m_position[0], _splitpoint3->m_position[2] }));
	}

	
	DCEL_Vertex* _a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin];
	DCEL_Vertex* _b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin];
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin];

	_a->m_meanGPR = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceGPR = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	//_a->m_meanLinear = m_triangle->getMean({ _a->m_position[0], _a->m_position[2] });
	//_a->m_varianceLinear = m_triangle->getVariance({ _a->m_position[0], _a->m_position[2] });
	_a->m_meanLinear = m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] });
	_a->m_varianceLinear = m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] });

	_b->m_meanGPR = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceGPR = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	//_b->m_meanLinear = m_triangle->getMean({ _b->m_position[0], _b->m_position[2] });
	//_b->m_varianceLinear = m_triangle->getVariance({ _b->m_position[0], _b->m_position[2] });
	_b->m_meanLinear = m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] });
	_b->m_varianceLinear = m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] });

	_c->m_meanGPR = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceGPR = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	//_c->m_meanLinear = m_triangle->getMean({ _c->m_position[0], _c->m_position[2] });
	//_c->m_varianceLinear = m_triangle->getVariance({ _c->m_position[0], _c->m_position[2] });
	_c->m_meanLinear = m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] });
	_c->m_varianceLinear = m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] });

	

	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];
	DCEL_HalfEdge* halfEdge6 = new DCEL_HalfEdge(m_dcel);
	halfEdge6->add(false);
	DCEL_HalfEdge* halfEdge7 = new DCEL_HalfEdge(m_dcel);
	halfEdge7->add(false);
	DCEL_HalfEdge* halfEdge8 = new DCEL_HalfEdge(m_dcel);
	halfEdge8->add(false);
	DCEL_HalfEdge* halfEdge9 = new DCEL_HalfEdge(m_dcel);
	halfEdge9->add(false);
	DCEL_HalfEdge* halfEdge10 = new DCEL_HalfEdge(m_dcel);
	halfEdge10->add(false);
	DCEL_HalfEdge* halfEdge11 = new DCEL_HalfEdge(m_dcel);
	halfEdge11->add(false);
	DCEL_HalfEdge* halfEdge12 = new DCEL_HalfEdge(m_dcel);
	halfEdge12->add(false);
	DCEL_HalfEdge *halfEdge13 = new DCEL_HalfEdge(m_dcel);
	halfEdge13->add(false);
	DCEL_HalfEdge *halfEdge14 = new DCEL_HalfEdge(m_dcel);
	halfEdge14->add(false);
	DCEL_HalfEdge* halfEdge15 = new DCEL_HalfEdge(m_dcel);
	halfEdge15->add(false);
	halfEdge15->m_origin = _a->m_id;
	halfEdge15->m_destination = _splitpoint1->m_id;
	

	DCEL_HalfEdge *halfEdge16 = new DCEL_HalfEdge(m_dcel);
	halfEdge16->add(false);
	halfEdge16->m_origin = _splitpoint1->m_id;
	halfEdge16->m_destination = _splitpoint3->m_id;
	

	DCEL_HalfEdge *halfEdge17 = new DCEL_HalfEdge(m_dcel);
	halfEdge17->add(false);
	halfEdge17->m_origin = _splitpoint3->m_id;
	halfEdge17->m_destination = _a->m_id;
	

	halfEdge15->m_next = halfEdge16->m_id;
	halfEdge15->m_previous = halfEdge17->m_id;
	halfEdge15->m_face = m_id;

	halfEdge16->m_next = halfEdge17->m_id;
	halfEdge16->m_previous = halfEdge15->m_id;
	halfEdge16->m_face = m_id;

	halfEdge17->m_next = halfEdge15->m_id;
	halfEdge17->m_previous = halfEdge16->m_id;
	halfEdge17->m_face = m_id;

	


	Triangle* triangle2 = new Triangle(_splitpoint1->m_position, _b->m_position, _splitpoint2->m_position,
		_splitpoint1->m_meanGPR,
		_b->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_splitpoint1->m_varianceGPR,
		_b->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);


	DCEL_Face* face2 = new DCEL_Face(triangle2, m_dcel);
	face2->addAll(_splitpoint1, _b, _splitpoint2, halfEdge6, halfEdge7, halfEdge8, false);


	Triangle* triangle3 = new Triangle(_splitpoint1->m_position, _splitpoint2->m_position, _splitpoint3->m_position,
		_splitpoint1->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_splitpoint3->m_meanGPR,
		_splitpoint1->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		_splitpoint3->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face3 = new DCEL_Face(triangle3, m_dcel);
	face3->addAll(_splitpoint1, _splitpoint2, _splitpoint3, halfEdge9, halfEdge10, halfEdge11, false);

	Triangle* triangle4 = new Triangle(_splitpoint3->m_position, _splitpoint2->m_position, _c->m_position,
		_splitpoint3->m_meanGPR,
		_splitpoint2->m_meanGPR,
		_c->m_meanGPR,
		_splitpoint3->m_varianceGPR,
		_splitpoint2->m_varianceGPR,
		_c->m_varianceGPR,
		m_triangle->subdivide,
		m_triangle->m_gpr);

	DCEL_Face* face4 = new DCEL_Face(triangle4, m_dcel);
	face4->addAll(_splitpoint3, _splitpoint2, _c, halfEdge12, halfEdge13, halfEdge14, false);

	//set childs

	int child1 = halfEdge15->m_id;
	int child2 = halfEdge6->m_id;
	int child3 = halfEdge7->m_id;
	int child4 = halfEdge13->m_id;
	int child5 = halfEdge14->m_id;
	int child6 = halfEdge17->m_id;

	halfEdge0->m_child1 = child1;
	halfEdge0->m_child2 = child2;
	

	halfEdge1->m_child1 = child3;
	halfEdge1->m_child2 = child4;
	

	halfEdge2->m_child1 = child5;
	halfEdge2->m_child2 = child6;
	

	// set twins Face1 
	halfEdge15->m_twin = halfEdge0->m_twin;
	halfEdge16->m_twin = halfEdge11->m_id;
	halfEdge17->m_twin = halfEdge2->m_twin;

	// set twins Face2
	halfEdge6->m_twin = halfEdge0->m_twin;
	halfEdge7->m_twin = halfEdge1->m_twin;
	halfEdge8->m_twin = halfEdge9->m_id;


	// set twins Face3
	halfEdge9->m_twin = halfEdge8->m_id;
	halfEdge10->m_twin = halfEdge12->m_id;
	halfEdge11->m_twin = halfEdge16->m_id;

	// set twins Face4
	halfEdge12->m_twin = halfEdge10->m_id;
	halfEdge13->m_twin = halfEdge1->m_twin;
	halfEdge14->m_twin = halfEdge2->m_twin;

	


	if (m_dcel->m_halfEdges[halfEdge0->m_twin]->m_child1 > -1){


		std::vector<int> tmp;

		halfEdge6->m_twin = m_dcel->m_halfEdges[halfEdge0->m_twin]->m_child1;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge0->m_twin]->m_child1]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge6->m_id;
		}
		tmp.clear();



		halfEdge15->m_twin = m_dcel->m_halfEdges[halfEdge0->m_twin]->m_child2;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge0->m_twin]->m_child2]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge15->m_id;
		}
		tmp.clear();
	}

	if (m_dcel->m_halfEdges[halfEdge1->m_twin]->m_child1 > -1){


		std::vector<int> tmp;

		halfEdge13->m_twin = m_dcel->m_halfEdges[halfEdge1->m_twin]->m_child1;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge1->m_twin]->m_child1]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge13->m_id;
		}
		tmp.clear();



		halfEdge7->m_twin = m_dcel->m_halfEdges[halfEdge1->m_twin]->m_child2;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge1->m_twin]->m_child2]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge7->m_id;
		}
		tmp.clear();

	}

	if (m_dcel->m_halfEdges[halfEdge2->m_twin]->m_child1 > -1){
		

		std::vector<int> tmp;

		halfEdge17->m_twin = m_dcel->m_halfEdges[halfEdge2->m_twin]->m_child1;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge2->m_twin]->m_child1]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
			
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge17->m_id;
		}
		tmp.clear();

		halfEdge14->m_twin = m_dcel->m_halfEdges[halfEdge2->m_twin]->m_child2;
		m_dcel->m_halfEdges[m_dcel->m_halfEdges[halfEdge2->m_twin]->m_child2]->getChilds(tmp);

		for (int i = 0; i < tmp.size(); i++){
	
			m_dcel->m_halfEdges[tmp[i]]->m_twin = halfEdge14->m_id;
		}
		tmp.clear();
	}

	// set face 1
	m_triangle->m_meanGPR1 = _a->m_meanGPR;
	m_triangle->m_meanGPR2 = _splitpoint1->m_meanGPR;
	m_triangle->m_meanGPR3 = _splitpoint3->m_meanGPR;
	m_triangle->m_varianceGPR1 = _a->m_varianceGPR;
	m_triangle->m_varianceGPR2 = _splitpoint1->m_varianceGPR;
	m_triangle->m_varianceGPR3 = _splitpoint3->m_varianceGPR;
	m_triangle->m_a = _a->m_position;
	m_triangle->m_b = _splitpoint1->m_position;
	m_triangle->m_c = _splitpoint3->m_position;
	m_triangle->errorCalaculated = false;
	m_edge = halfEdge15->m_id;

	m_dcel->q.pop();
	face2->m_triangle->calcSurfaceError(m_dcel->m_grid, 100);
	face3->m_triangle->calcSurfaceError(m_dcel->m_grid, 100);
	face4->m_triangle->calcSurfaceError(m_dcel->m_grid, 100);
	m_triangle->calcSurfaceError(m_dcel->m_grid, 100);


	if (m_triangle->subdivide){
		m_dcel->q.push(this);
	}

	if (face2->m_triangle->subdivide){
		m_dcel->q.push(face2);
	}

	if (face3->m_triangle->subdivide){
		m_dcel->q.push(face3);
	}

	if (face4->m_triangle->subdivide){
		m_dcel->q.push(face4);
	}
	

}


void DCEL_Face::getNeighbours(int level){

	std::set<int> neighbours;
	
	if (level <= 0){

		m_neighbours.push_back(m_id);
		
		return;
	}


	neighbours.insert(m_id);
	getNeighbours(neighbours, level-1);

	std::copy(neighbours.begin(), neighbours.end(), std::back_inserter(m_neighbours));
	neighbours.clear();

}

void DCEL_Face::getNeighbours(std::set<int>& a_neighbours, int level){
	

	if (level <= -1){
		
		return;
	}

	std::set<int>::iterator it = a_neighbours.begin();
	int end = a_neighbours.size();

	std::set<int> tmp;
	for (int i = 0; i < a_neighbours.size(); i++, it++){
		
		
		
		int begin = m_dcel->m_faces[*it]->m_edge;
			int next1 = m_dcel->m_halfEdges[begin]->m_next;
			int next2 = m_dcel->m_halfEdges[next1]->m_next;

			if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin]->m_face != INT_MAX){
				
				tmp.insert(m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin]->m_face);

			}

			if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin]->m_face != INT_MAX){

				tmp.insert(m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin]->m_face);
			}

			if (m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin]->m_face != INT_MAX){
				

				tmp.insert(m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin]->m_face);
			}
			
	}

	a_neighbours.insert(tmp.begin(), tmp.end());

	getNeighbours(a_neighbours, level - 1);

}

void DCEL_Face::buildFans(){

	int begin = m_edge;
	int next1 = m_dcel->m_halfEdges[begin]->m_next;
	int next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;

	std::vector<int> indices;
	std::vector<int> indicesToErase;


	DCEL_HalfEdge* halfEdge0 = m_dcel->m_halfEdges[begin];
	DCEL_HalfEdge* halfEdge1 = m_dcel->m_halfEdges[next1];
	DCEL_HalfEdge* halfEdge2 = m_dcel->m_halfEdges[next2];

	DCEL_HalfEdge* twin0 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_twin];
	DCEL_HalfEdge* twin1 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[next1]->m_twin];
	DCEL_HalfEdge* twin2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[next2]->m_twin];
	
	if (twin0->m_child1 > 0 &&
		m_dcel->m_halfEdges[twin0->m_child1]->m_origin == halfEdge0->m_destination &&
		m_dcel->m_halfEdges[twin0->m_child2]->m_destination == halfEdge0->m_origin){

		indices.push_back(twin0->m_twin);
		getChilds(twin0);

	}else{

		indicesToErase.push_back(begin);
		m_childs.push_back(begin);
	}

	if (twin1->m_child1 > 0 &&
		m_dcel->m_halfEdges[twin1->m_child1]->m_origin == halfEdge1->m_destination &&
		m_dcel->m_halfEdges[twin1->m_child2]->m_destination == halfEdge1->m_origin){

		indices.push_back(twin1->m_twin);
		getChilds(twin1);

	}else{

		indicesToErase.push_back(next1);
		m_childs.push_back(next1);
	}

	if (twin2->m_child1 > 0 &&
		m_dcel->m_halfEdges[twin2->m_child1]->m_origin == halfEdge2->m_destination &&
		m_dcel->m_halfEdges[twin2->m_child2]->m_destination == halfEdge2->m_origin
		){

		indices.push_back(twin2->m_twin);
		getChilds(twin2);

	}else{

		indicesToErase.push_back(next2);
		m_childs.push_back(next2);
	}

	// one or two edges are splitted
	if (indices.size() > 1){

		_splitFan();

		// one edge is splitted
	}
	else if (indices.size() == 1){

		m_childs.erase(std::remove(m_childs.begin(), m_childs.end(), indicesToErase[0]), m_childs.end());
		m_childs.erase(std::remove(m_childs.begin(), m_childs.end(), indicesToErase[1]), m_childs.end());
		_splitFan(indices[0]);
	}


	
	
}

void DCEL_Face::getChilds(DCEL_HalfEdge* halfEdge){
	
	if (halfEdge->m_child1 == -1){
		
		m_childs.push_back(halfEdge->m_id);

	}else{
	 
		getChilds(m_dcel->m_halfEdges[halfEdge->m_child2]);
		getChilds(m_dcel->m_halfEdges[halfEdge->m_child1]);
	}


}





void DCEL_Face::_splitFan(){

	Vector3f midpoint = (m_triangle->m_a + m_triangle->m_b + m_triangle->m_c) / 3.0;
	DCEL_Vertex* _midpoint = new DCEL_Vertex(midpoint, m_dcel);
	_midpoint->add(false);

	//_midpoint->m_meanLinear = (m_triangle->m_meanLinear1 + m_triangle->m_meanLinear2 + m_triangle->m_meanLinear3)/3.0;
	//_midpoint->m_varianceLinear = (m_triangle->m_varianceLinear1 + m_triangle->m_varianceLinear2 + m_triangle->m_varianceLinear3) / 3.0;
	//_midpoint->m_meanLinear = m_triangle->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	//_midpoint->m_varianceLinear = m_triangle->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });

	_midpoint->m_meanGPR = m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_varianceGPR = m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_meanLinear = m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] });
	_midpoint->m_varianceLinear = m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] });

	DCEL_HalfEdge* halfEdge0;
	DCEL_HalfEdge* halfEdge1;
	DCEL_HalfEdge* halfEdge2;

	DCEL_Vertex* _a;
	DCEL_Vertex* _b;

	std::vector<int> twins;
	std::vector<int> twins2;

	for (int i = 0; i < m_childs.size(); i++){
		
		halfEdge0 = new DCEL_HalfEdge(m_dcel);
		halfEdge0->add(false);
		//halfEdge0->m_twin = m_childs[i];
		//m_dcel->m_halfEdges[m_childs[i]]->m_twin = halfEdge0->m_id;
		halfEdge1 = new DCEL_HalfEdge(m_dcel);
		halfEdge1->add(false);
		halfEdge2 = new DCEL_HalfEdge(m_dcel);
		halfEdge2->add(false);

		twins.push_back(halfEdge1->m_id);
		twins.push_back(halfEdge2->m_id);

		_a = m_dcel->m_vertices[m_dcel->m_halfEdges[m_childs[i]]->m_destination];
		_b = m_dcel->m_vertices[m_dcel->m_halfEdges[m_childs[i]]->m_origin];

		Triangle* triangle = new Triangle(_a->m_position, _b->m_position, _midpoint->m_position,
			m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] }),
			m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] }),
			m_triangle->m_gpr->getMean({ _midpoint->m_position[0], _midpoint->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _midpoint->m_position[0], _midpoint->m_position[2] }),
			m_triangle->subdivide,
			m_triangle->m_gpr);

		DCEL_Face* face = new DCEL_Face(triangle, m_dcel);
		face->addAll(_a, _b, _midpoint, halfEdge0, halfEdge1, halfEdge2, false);
	}
	render = false;
	
	m_childs.clear();
}

void DCEL_Face::_splitFan(int index){

	DCEL_HalfEdge* halfEdge0;
	DCEL_HalfEdge* halfEdge1;
	DCEL_HalfEdge* halfEdge2;

	DCEL_Vertex* _a;
	DCEL_Vertex* _b;
	DCEL_Vertex* _c = m_dcel->m_vertices[m_dcel->m_halfEdges[m_dcel->m_halfEdges[index]->m_next]->m_destination];

	std::vector<int> twins;
	std::vector<int> twins2;
	
	for (int i = 0; i < m_childs.size(); i++){

		halfEdge0 = new DCEL_HalfEdge(m_dcel);
		halfEdge0->add(false);
		//halfEdge0->m_twin = m_childs[i];
		//m_dcel->m_halfEdges[m_childs[i]]->m_twin = halfEdge0->m_id;
		halfEdge1 = new DCEL_HalfEdge(m_dcel);
		halfEdge1->add(false);
		halfEdge2 = new DCEL_HalfEdge(m_dcel);
		halfEdge2->add(false);

		twins.push_back(halfEdge1->m_id);
		twins.push_back(halfEdge2->m_id);

		_a = m_dcel->m_vertices[m_dcel->m_halfEdges[m_childs[i]]->m_destination];
		_b = m_dcel->m_vertices[m_dcel->m_halfEdges[m_childs[i]]->m_origin];

		Triangle* triangle = new Triangle(_a->m_position, _b->m_position, _c->m_position,
			m_triangle->m_gpr->getMean({ _a->m_position[0], _a->m_position[2] }),
			m_triangle->m_gpr->getMean({ _b->m_position[0], _b->m_position[2] }),
			m_triangle->m_gpr->getMean({ _c->m_position[0], _c->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _a->m_position[0], _a->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _b->m_position[0], _b->m_position[2] }),
			m_triangle->m_gpr->getVariance({ _c->m_position[0], _c->m_position[2] }),
			m_triangle->subdivide,
			m_triangle->m_gpr);

		DCEL_Face* face = new DCEL_Face(triangle, m_dcel);
		face->addAll(_a, _b, _c, halfEdge0, halfEdge1, halfEdge2, false);
	}
	render = false;
	m_childs.clear();
}


void DCEL_Face::faceToTriangle(){

	int begin = m_edge;
	int next1 = m_dcel->m_halfEdges[begin]->m_next;
	int next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;

	Vector3f a = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_position;
	Vector3f b = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin]->m_position;
	Vector3f c = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin]->m_position;

	double  meanGPR1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_meanGPR;
	double  meanGPR2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin]->m_meanGPR;
	double  meanGPR3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin]->m_meanGPR;
	double  varianceGPR1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_varianceGPR;
	double  varianceGPR2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin]->m_varianceGPR;
	double  varianceGPR3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin]->m_varianceGPR;
	double  meanLinear1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_meanLinear;
	double  meanLinear2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin]->m_meanLinear;
	double  meanLinear3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin]->m_meanLinear;
	double  varianceLinear1 = m_dcel->m_vertices[m_dcel->m_halfEdges[begin]->m_origin]->m_varianceLinear;
	double  varianceLinear2 = m_dcel->m_vertices[m_dcel->m_halfEdges[next1]->m_origin]->m_varianceLinear;
	double  varianceLinear3 = m_dcel->m_vertices[m_dcel->m_halfEdges[next2]->m_origin]->m_varianceLinear;

	

	m_dcel->m_grid->addFacetoBuffer(m_triangle, a, b, c,
		meanGPR1, meanGPR2, meanGPR3,
		varianceGPR1, varianceGPR2, varianceGPR3,
		meanLinear1, meanLinear2, meanLinear3,
		varianceLinear1, varianceLinear2, varianceLinear3);
}

void DCEL_Face::setGPR(int radius, double sigma_p, double l){

	getNeighbours(radius);
	std::set<int> vertices;
	int begin, next1, next2;
	std::vector<std::tuple<std::vector<double>, double, double>> trainData;

	for (int i = 0; i < m_neighbours.size(); i++){

		begin = m_dcel->m_faces[m_neighbours[i]]->m_edge;
		vertices.insert(m_dcel->m_halfEdges[begin]->m_origin);

		next1 = m_dcel->m_halfEdges[begin]->m_next;
		vertices.insert(m_dcel->m_halfEdges[next1]->m_origin);

		next2 = m_dcel->m_halfEdges[m_dcel->m_halfEdges[begin]->m_next]->m_next;
		vertices.insert(m_dcel->m_halfEdges[next2]->m_origin);

	}
	std::set<int>::iterator it;
	for (it = vertices.begin(); it != vertices.end(); ++it){

		trainData.push_back(std::tuple<std::vector<double>, double, double>({ m_dcel->m_vertices[*it]->m_position[0], m_dcel->m_vertices[*it]->m_position[2] }, m_dcel->m_vertices[*it]->m_meanGPR, m_dcel->m_vertices[*it]->m_varianceGPR));
	}
	m_triangle->m_gpr = new GPR(trainData, sigma_p, l);

	m_triangle->m_meanGPR1 = m_triangle->m_gpr->getMean({ m_triangle->m_a[0], m_triangle->m_a[2] });
	m_triangle->m_meanGPR2 = m_triangle->m_gpr->getMean({ m_triangle->m_b[0], m_triangle->m_b[2] });
	m_triangle->m_meanGPR3 = m_triangle->m_gpr->getMean({ m_triangle->m_c[0], m_triangle->m_c[2] });
	m_triangle->m_varianceGPR1 = m_triangle->m_gpr->getVariance({ m_triangle->m_a[0], m_triangle->m_a[2] });
	m_triangle->m_varianceGPR2 = m_triangle->m_gpr->getVariance({ m_triangle->m_b[0], m_triangle->m_b[2] });
	m_triangle->m_varianceGPR3 = m_triangle->m_gpr->getVariance({ m_triangle->m_c[0], m_triangle->m_c[2] });

	//std::cout << m_triangle->m_varianceGPR1 << std::endl;

	//first parameter stores the maximal grid error
	m_triangle->calcSurfaceError(m_dcel->m_grid, 100);
	
	if (m_triangle->subdivide){

		m_dcel->q.push(this);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

void DCEL::addFacesToBuffer(){

	m_grid->clearBuffer();
	for (int i = 0; i < m_faces.size(); i++){

		if (m_faces[i]->render){

			m_faces[i]->faceToTriangle();
			//std::cout << "Cholesky Face: "<< std::endl;
			//m_faces[i]->m_triangle->m_gpr->printCholesky();
		}
	}

	

	m_grid->bindBuffer();
}

void DCEL::calcGlobalError(){

	globalMean = 0.0;
	globalBha = 0.0;

	for (int i = 0; i < m_faces.size(); i++){

		globalMean = globalMean + m_faces[i]->m_triangle->meanError;
		globalBha = globalBha + m_faces[i]->m_triangle->bhaError;
	}

	mean = q.top()->m_triangle->meanError;
	bha = q.top()->m_triangle->bhaError;
}


void DCEL::calcFaceErrors(){

	
	double tmpMean;
	double tmpBha;

	if (m_faces.size() > 0){

		m_face = m_faces[0];
		m_faces[0]->m_triangle->calcSurfaceError(m_grid, 100);

		mean = m_faces[0]->m_triangle->meanError;
		bha = m_faces[0]->m_triangle->bhaError;

		globalMean = mean;
		globalBha = bha;
	}


	for (int i = 1; i < m_faces.size(); i++){
	
		m_faces[i]->m_triangle->calcSurfaceError(m_grid, 100);

		 tmpMean = m_faces[i]->m_triangle->meanError;
		 tmpBha = m_faces[i]->m_triangle->bhaError;

		 globalMean = globalMean + tmpMean;
		 globalBha = globalBha + tmpBha;

		 

		 /*if (tmpMean > mean){
			 mean = tmpMean;
			m_face = m_faces[i];
		 }*/


		 if (tmpBha > bha){
			 bha = tmpBha;
			 m_face = m_faces[i];
		 }
	}
}

void DCEL::calcSplitPoints(){

	for (int i = 0; i < m_halfEdges.size(); i++){

		// avoid numerical inaccuracy and multiple splitpoints
		if (m_halfEdges[i]->m_face < INT_MAX  &&  m_halfEdges[m_halfEdges[i]->m_twin]->m_splitpoint.calculated &&  m_faces[m_halfEdges[m_halfEdges[i]->m_twin]->m_face]->m_triangle->subdivide){
			
			m_halfEdges[i]->m_splitpoint = m_halfEdges[m_halfEdges[i]->m_twin]->m_splitpoint;

		}else if (m_halfEdges[i]->m_face < INT_MAX){

			
			m_halfEdges[i]->calcSplitpoint( m_faces[m_halfEdges[i]->m_face]);
		}
	}
}





void DCEL::split(){

	int numberOfFaces = m_faces.size();

	for (int i = 0; i < numberOfFaces; i++){

		std::vector<int> indices;

		int begin = m_faces[i]->m_edge;
		int next = begin;

		do{

			if (m_halfEdges[m_halfEdges[next]->m_twin]->m_face != INT_MAX && m_faces[m_halfEdges[m_halfEdges[next]->m_twin]->m_face]->m_triangle->subdivide  ){

				indices.push_back(next);
			}
			
			next = m_halfEdges[next]->m_next;
			
		} while (next != begin);
		
		if (m_faces[i]->m_triangle->subdivide){
		
			m_faces[i]->split4();

		}else if (!m_faces[i]->m_triangle->subdivide && indices.size() == 3){
			
			m_faces[i]->split6();
		
		}else if (!m_faces[i]->m_triangle->subdivide && indices.size() == 2){
			
			//ABC
			if (m_halfEdges[m_halfEdges[begin]->m_twin]->m_face != INT_MAX &&
				m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_twin]->m_face != INT_MAX &&
				m_faces[m_halfEdges[m_halfEdges[begin]->m_twin]->m_face]->m_triangle->subdivide &&
				m_faces[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_twin]->m_face]->m_triangle->subdivide){
			
				m_faces[i]->split3(indices[0], indices[1]);
			//BCA
			}else if (m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_twin]->m_face != INT_MAX &&
				m_halfEdges[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_next]->m_twin]->m_face != INT_MAX &&
				m_faces[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_twin]->m_face]->m_triangle->subdivide &&
				m_faces[m_halfEdges[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_next]->m_twin]->m_face]->m_triangle->subdivide){
			
				m_faces[i]->split3(indices[0], indices[1]);

			//CAB
			}else if (m_halfEdges[m_halfEdges[begin]->m_twin]->m_face != INT_MAX && 
				m_halfEdges[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_next]->m_twin]->m_face != INT_MAX &&
				m_faces[m_halfEdges[m_halfEdges[begin]->m_twin]->m_face]->m_triangle->subdivide &&
				m_faces[m_halfEdges[m_halfEdges[m_halfEdges[m_halfEdges[begin]->m_next]->m_next]->m_twin]->m_face]->m_triangle->subdivide){
				
				m_faces[i]->split3(indices[1], indices[0]);

			}

		}else if (!m_faces[i]->m_triangle->subdivide && indices.size() == 1){
			
			m_faces[i]->split2(indices[0]);
		}
	}
}

void DCEL::_split(){

	
	q.top()->_split4();

	
}

void DCEL::_splitFan(){


	int numberOfFaces = m_faces.size();
	for (int i = 0; i < numberOfFaces; i++){
		
		m_faces[i]->buildFans();
	}

}

void DCEL::buildDCEL(const char* filename){

	std::vector<double> means;
	std::vector<double> variances;
	std::vector<std::array<int, 3>> faces;

	std::ifstream ifs(filename);

	if (ifs.is_open()) {

		std::string inputLine;

		while (std::getline(ifs, inputLine)){

			char identifier[20];
			int numVertices;
			char dataType[20];
			int numCells;

			char kind[20];
			char name[20];

			sscanf(inputLine.c_str(), "%s", &identifier);

			if (inputLine[0] == 'P') {

				sscanf(inputLine.c_str(), "%s %d %s", &identifier, &numVertices, &dataType);
				std::cout << identifier << " " << numVertices << " " << dataType << std::endl;

				double posx, posy, posz, scalar;

				if (std::string(identifier) == "POINTS"){

					for (int i = 0; i < numVertices; i++) {

						ifs >> posx >> posy >> posz;

						DCEL_Vertex* vertex = new DCEL_Vertex(Vector3f(posx, posz, posy), this);
						vertex->add(false);
					}
				}else if (std::string(identifier) == "POINT_DATA"){

					std::getline(ifs, inputLine);
					sscanf(inputLine.c_str(), "%s %s %s", &kind, &name, &dataType);
					std::cout << kind << "  " << name << "  " << std::endl;
					ifs.ignore(50, '\n');

					if (std::string(name) == "mean"){

						for (int i = 0; i < numVertices; i++) {

							ifs >> scalar;
							means.push_back(scalar);
						}

					}else if (std::string(name) == "variance"){

						for (int i = 0; i < numVertices; i++) {

							ifs >> scalar;
							variances.push_back(scalar);
						}
					}
				}// PONT_DATA

				}else if (inputLine[0] == 'C'){

					sscanf(inputLine.c_str(), "%s %d", &identifier, &numCells);

					if (std::string(identifier) == "CELLS"){

						std::cout << identifier << " " << numCells << std::endl;
						int verticesPerCell, index1, index2, index3;
					
						for (int i = 0; i < numCells; i++) {

							ifs >> verticesPerCell >> index1 >> index2 >> index3;

							faces.push_back({ { index1, index2, index3 } });
						}


					}// CELLS
				}//C

			}
		}
		
		ifs.close();
	
	for (int i = 0; i < m_vertices.size(); i++){

		m_vertices[i]->m_meanGPR		= means[i] ;
		m_vertices[i]->m_varianceGPR    = variances[i];
		m_vertices[i]->m_meanLinear = means[i] ;
		m_vertices[i]->m_varianceLinear = variances[i];

		
	}
	

	for (int i = 0; i < faces.size(); i++){

		Triangle* triangle = new Triangle(m_vertices[(faces[i])[0]]->m_position, m_vertices[(faces[i])[1]]->m_position, m_vertices[(faces[i])[2]]->m_position,
			means[(faces[i])[0]], means[(faces[i])[1]], means[(faces[i])[2]],
			variances[(faces[i])[0]], variances[(faces[i])[1]], variances[(faces[i])[2]], true);

		DCEL_Face* face = new DCEL_Face(triangle, this);
		face->addAll(m_vertices[(faces[i])[0]], m_vertices[(faces[i])[1]], m_vertices[(faces[i])[2]], NULL, NULL, NULL, false);

	}
	findTwins();
}


void DCEL::buildDCEL(std::vector<Triangle*> a_triangles){

	for (int i = 0; i < a_triangles.size(); i++){

		DCEL_Vertex* _a = new DCEL_Vertex(a_triangles[i]->m_a, this);
		_a->add(true);
	
		DCEL_Vertex* _b = new DCEL_Vertex(a_triangles[i]->m_b, this);
		_b->add(true);
		
		DCEL_Vertex* _c = new DCEL_Vertex(a_triangles[i]->m_c, this);
		_c->add(true);
		
		DCEL_Face* face = new DCEL_Face(a_triangles[i], this);
		face->addAll(_a, _b, _c, NULL, NULL, NULL, false);
	}

	findTwins();
}


bool compareHalfEdge(DCEL_HalfEdge* const& lhs, DCEL_HalfEdge const& rhs){
	
	return lhs->m_origin == rhs.m_destination && lhs->m_destination == rhs.m_origin ;
}

void DCEL::findTwins(){

	if (m_halfEdges.empty()) return;

	std::vector<DCEL_HalfEdge*> tmpHalfEdges;

	copy(m_halfEdges.begin(), m_halfEdges.end(), back_inserter(tmpHalfEdges));
	DCEL_HalfEdge* halfEdge;
	
	do{
		
		DCEL_HalfEdge* ref = tmpHalfEdges[0];

		auto it = std::find_if(tmpHalfEdges.begin(), tmpHalfEdges.end(), [ref](DCEL_HalfEdge* lhs) { return lhs->m_origin == ref->m_destination && lhs->m_destination == ref->m_origin; });


		if (it != tmpHalfEdges.end()){
			int index = std::distance(tmpHalfEdges.begin(), it);
			tmpHalfEdges[0]->m_twin = tmpHalfEdges[index]->m_id;
			tmpHalfEdges[index]->m_twin = tmpHalfEdges[0]->m_id;

			tmpHalfEdges.erase(it);
			tmpHalfEdges.erase(tmpHalfEdges.begin());

		}else{

			halfEdge = new DCEL_HalfEdge(this);
			halfEdge->add(false);

			halfEdge->m_origin = tmpHalfEdges[0]->m_destination;
			halfEdge->m_destination = tmpHalfEdges[0]->m_origin;
			halfEdge->m_face = INT_MAX;
			halfEdge->m_twin = tmpHalfEdges[0]->m_id;
			tmpHalfEdges[0]->m_twin = halfEdge->m_id;
			halfEdge->m_border = true;
			tmpHalfEdges.erase(tmpHalfEdges.begin());
		}

	} while (tmpHalfEdges.size() > 0);

	
}


void DCEL::setGPR(int radius, double sigma_p, double l){

	//fisrt step: Set up the GPR for every triangle
	for (int i = 0; i < m_faces.size(); i++){

		m_faces[i]->setGPR(radius, sigma_p, l);
	}

	//second step calculate the means and variances for every DCEL_Vertex to render the initial load of the data
	int begin;
	int next1;
	int next2;

	DCEL_Vertex* a;
	DCEL_Vertex* b;
	DCEL_Vertex* c;

	
	for (int i = 0; i < m_faces.size(); i++){
	
		begin = m_faces[i]->m_edge;
		next1 = m_halfEdges[begin]->m_next;
		next2 = m_halfEdges[m_halfEdges[begin]->m_next]->m_next;

		a = m_vertices[m_halfEdges[begin]->m_origin];
		a->setMeanGPR(m_faces[i]->m_triangle->m_gpr->getMean({ a->m_position[0], a->m_position[2] }));
		a->setVarianceGPR(m_faces[i]->m_triangle->m_gpr->getVariance({ a->m_position[0], a->m_position[2] }));
		a->setMeanLinear(m_faces[i]->m_triangle->m_gpr->getMean({ a->m_position[0], a->m_position[2] }));
		a->setVarianceLinear(m_faces[i]->m_triangle->m_gpr->getVariance({ a->m_position[0], a->m_position[2] }));

		b = m_vertices[m_halfEdges[next1]->m_origin];
		b->setMeanGPR(m_faces[i]->m_triangle->m_gpr->getMean({ b->m_position[0], b->m_position[2] }));
		b->setVarianceGPR(m_faces[i]->m_triangle->m_gpr->getVariance({ b->m_position[0], b->m_position[2] }));
		b->setMeanLinear(m_faces[i]->m_triangle->m_gpr->getMean({ b->m_position[0], b->m_position[2] }));
		b->setVarianceLinear(m_faces[i]->m_triangle->m_gpr->getVariance({ b->m_position[0], b->m_position[2] }));
		
		c = m_vertices[m_halfEdges[next2]->m_origin];
		c->setMeanGPR(m_faces[i]->m_triangle->m_gpr->getMean({ c->m_position[0], c->m_position[2] }));
		c->setVarianceGPR(m_faces[i]->m_triangle->m_gpr->getVariance({ c->m_position[0], c->m_position[2] }));
		c->setMeanLinear(m_faces[i]->m_triangle->m_gpr->getMean({ c->m_position[0], c->m_position[2] }));
		c->setVarianceLinear(m_faces[i]->m_triangle->m_gpr->getVariance({ c->m_position[0], c->m_position[2] }));	
	}

}



