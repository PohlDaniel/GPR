#ifndef _DCEL_H
#define _DCEL_H

#include <vector>
#include <memory>
#include <unordered_set>
#include <queue>
#include <functional>

#include "Vector.h"
#include "TriangleGrid.h"

class DCEL_Face;

struct ComparePtr{

	bool operator()(DCEL_Face const* lhs, DCEL_Face const* rhs); 
};


class DCEL {

	friend struct DCEL_Vertex;
	friend struct DCEL_HalfEdge;
	friend struct DCEL_Face;

public:

	DCEL(TriangleGrid* a_grid){

		m_grid = a_grid;
	}

	void buildDCEL(std::vector<Triangle*> triangles);
	void buildDCEL(const char* filename);
	void calcFaceErrors();
	void calcSplitPoints();
	void calcGlobalError();

	void findTwins();

	void split();
	void _split();
	void _splitFan();

	void addFacesToBuffer();

	void setGPR(int radius, double sigma_p, double l);

	TriangleGrid * m_grid;

	double mean = 0.0;
	double bha = 0.0;

	double globalMean = 0.0;
	double globalBha = 0.0;

	DCEL_Face *m_face = NULL;

	std::vector<DCEL_Vertex*> m_vertices;
	std::vector<DCEL_HalfEdge*> m_halfEdges;
	std::vector<DCEL_Face*> m_faces;
	
	std::priority_queue< DCEL_Face*, std::vector< DCEL_Face* >, ComparePtr > q;
};



struct DCEL_Vertex{

	friend bool compareVertex(DCEL_Vertex* const& lhs, DCEL_Vertex const& rhs);

	DCEL_Vertex(Vector3f& position, DCEL* dcel){

		m_position = position;
		m_dcel = dcel;
		m_id = -1;
	
	}

	int found(DCEL& dcel);

	bool add(bool dublicate){
		
		if (dublicate){
			
			if (m_id > -1) return true;

			//add the vertex in case the DCEL doesn't hold it
			int index = found(*m_dcel);
			
			if (index == -1){
				
				m_dcel->m_vertices.push_back(this);
				m_id = m_dcel->m_vertices.size() - 1;
				return false;
			}

			m_id = index;

			return true;

		}else{
			

			m_dcel->m_vertices.push_back(this);
			m_id = m_dcel->m_vertices.size() - 1;
			return false;
		}

	}

	DCEL* m_dcel;
	Vector3f m_position;
	int m_id;

	double m_meanGPR = FLT_MAX;
	double m_varianceGPR = FLT_MAX;
	double m_meanLinear = FLT_MAX;
	double m_varianceLinear = FLT_MAX;


	std::vector<double> m_meansGPR;
	std::vector<double> m_variancesGPR;
	std::vector<double> m_meansLinear;
	std::vector<double> m_variancesLinear;

	void setMeanGPR(double a_mean){

		m_meansGPR.push_back(a_mean);
		m_meanGPR = 0.0;
		for (int i = 0; i < m_meansGPR.size(); i++){

			m_meanGPR = m_meanGPR + m_meansGPR[i];
		}

		m_meanGPR = m_meanGPR / m_meansGPR.size();

	}

	void setVarianceGPR(double a_variance){

		m_variancesGPR.push_back(a_variance);
		m_varianceGPR = 0.0;
		for (int i = 0; i < m_variancesGPR.size(); i++){

			m_varianceGPR = m_varianceGPR + m_variancesGPR[i];
		}

		m_varianceGPR = m_varianceGPR / m_variancesGPR.size();

	}


	void setMeanLinear(double a_mean){

		m_meansLinear.push_back(a_mean);
		m_meanLinear = 0.0;
		for (int i = 0; i < m_meansLinear.size(); i++){

			m_meanLinear = m_meanLinear + m_meansLinear[i];
		}

		m_meanLinear = m_meanLinear / m_meansLinear.size();

	}


	void setVarianceLinear(double a_variance){

		m_variancesLinear.push_back(a_variance);
		m_varianceLinear = 0.0;
		for (int i = 0; i < m_variancesLinear.size(); i++){

			m_varianceLinear = m_varianceLinear + m_variancesLinear[i];
		}

		m_varianceLinear = m_varianceLinear / m_variancesLinear.size();

	}

};


struct DCEL_HalfEdge{

	friend bool compareHalfEdge(DCEL_HalfEdge* const& lhs, DCEL_HalfEdge const& rhs);
	
	DCEL_HalfEdge(DCEL* dcel){

		m_dcel = dcel;
		m_id = -1;

		m_origin = -1;
		m_destination = -1;
		m_face = -1;
		m_twin = -1;
		m_twin2 = -1;
		m_child1 = -1;
		m_child2 = -1;
		m_next = -1;
		m_previous = -1;
		m_border = false;
	}

	int found(DCEL& dcel);

	bool add(bool dublicate){
		if (dublicate){
			
			if (m_id > -1) return true;

			//add the edge in case the DCEL doesn't hold it
			if (found(*m_dcel) == -1){

				m_dcel->m_halfEdges.push_back(this);
				m_id = m_dcel->m_halfEdges.size() - 1;

				return false;
			}

			return true;

		}else{

			m_dcel->m_halfEdges.push_back(this);
			m_id = m_dcel->m_halfEdges.size() - 1;
			return false;
		}
	}

	void calcSplitpoint(DCEL_Face* face);
	
	DCEL* m_dcel;
	int m_id;

	struct splitpoint{
		Vector3f pos;
		int index = -1;
		bool calculated = false;
	};
	
	
	void getChilds(std::vector<int> &childs);

	splitpoint m_splitpoint;

	int m_origin;
	int m_destination;
	int m_face;

	int m_twin;
	int m_twin2;
	int m_child1;
	int m_child2;

	int m_next;
	int m_previous;

	bool m_border;
	
	
};



struct DCEL_Face{

	
	friend bool compareFace(DCEL_Face* const& lhs, DCEL_Face const& rhs);

	DCEL_Face(Triangle* triangle, DCEL* dcel){
		m_triangle = triangle;
		m_dcel = dcel;
		m_id = -1;
		render = true;
	}
	
	int found(DCEL& dcel);

	bool addAll(DCEL_Vertex* a_vertex1, DCEL_Vertex* a_vertex2, DCEL_Vertex* a_vertex3, DCEL_HalfEdge *a_halfEdge1, DCEL_HalfEdge *a_halfEdge2, DCEL_HalfEdge *a_halfEdge3, bool duplicate){

		if (m_id > -1) return true;

		DCEL_Vertex* vertex1;
		DCEL_Vertex* vertex2;
		DCEL_Vertex* vertex3;

		if (a_vertex1){
		
			vertex1 = a_vertex1;
		
		}else{
		
			vertex1 = new DCEL_Vertex(m_triangle->m_a, m_dcel);
			vertex1->add(true);
			
		}

		if (a_vertex2){

			vertex2 = a_vertex2;

		}else{

			vertex2 = new DCEL_Vertex(m_triangle->m_b, m_dcel);
			vertex2->add(true);
		}

		if (a_vertex3){

			vertex3 = a_vertex3;

		}else{

			vertex3 = new DCEL_Vertex(m_triangle->m_c, m_dcel);
			vertex3->add(true);
		}

		DCEL_HalfEdge *halfEdge1;
		DCEL_HalfEdge *halfEdge2;
		DCEL_HalfEdge *halfEdge3;
		

		if (a_halfEdge1){

			halfEdge1 = a_halfEdge1;

		}else{
			
			halfEdge1 = new DCEL_HalfEdge(m_dcel);
			halfEdge1->add(false);
		}

		if (a_halfEdge2){

			halfEdge2 = a_halfEdge2;

		}else{

			halfEdge2 = new DCEL_HalfEdge( m_dcel);
			halfEdge2->add(false);
		}

		if (a_halfEdge3){

			halfEdge3 = a_halfEdge3;

		}else{
			
			halfEdge3 = new DCEL_HalfEdge(m_dcel);
			halfEdge3->add(false);
		}

		halfEdge1->m_origin = vertex1->m_id;
		halfEdge1->m_destination = vertex2->m_id;

		halfEdge2->m_origin = vertex2->m_id;
		halfEdge2->m_destination = vertex3->m_id;

		halfEdge3->m_origin = vertex3->m_id;
		halfEdge3->m_destination = vertex1->m_id;

		halfEdge1->m_next = halfEdge2->m_id;
		halfEdge2->m_next = halfEdge3->m_id;
		halfEdge3->m_next = halfEdge1->m_id;

		halfEdge1->m_previous = halfEdge3->m_id;
		halfEdge2->m_previous = halfEdge1->m_id;
		halfEdge3->m_previous = halfEdge2->m_id;

		if (duplicate){

			if (found(*m_dcel) == -1){

					m_dcel->m_faces.push_back(this);
					m_id = m_dcel->m_faces.size() - 1;
					m_edge = halfEdge1->m_id;

					halfEdge1->m_face = m_id;
					halfEdge2->m_face = m_id;
					halfEdge3->m_face = m_id;
					return false;
			}else{

					m_dcel->m_faces.push_back(this);
					m_id = m_dcel->m_faces.size() - 1;
					m_edge = halfEdge1->m_id;

					halfEdge1->m_face = m_id;
					halfEdge2->m_face = m_id;
					halfEdge3->m_face = m_id;
					return true;
			}
			
		}else{
				m_dcel->m_faces.push_back(this);
				m_id = m_dcel->m_faces.size() - 1;
				m_edge = halfEdge1->m_id;

				halfEdge1->m_face = m_id;
				halfEdge2->m_face = m_id;
				halfEdge3->m_face = m_id;

				return false;
		}

		
	}


	bool inline add(){

		m_dcel->m_faces.push_back(this);
		m_id = m_dcel->m_faces.size() - 1;
		return false;
	}
	

	std::vector<int> m_neighbours;
	std::vector<int> m_childs;
	
	void split2(int index);
	void split3(int index1, int index2);
	void split4();
	void split6();
	void getNeighbours(int level);
	void getNeighbours(std::set<int>& neighbours, int level);
	void setGPR(int radius, double sigma_p, double l);
	void showEdges();

	void buildFans();
	void getChilds(DCEL_HalfEdge* halfEdge);

	void _split4();
	void _splitFan();
	void _splitFan(int index);
	
	void faceToTriangle();
	
	
	DCEL* m_dcel;
	int m_id;
	
	int m_edge;

	///////////////////////////////////////////
	Triangle* m_triangle;
	bool render;

};
#endif