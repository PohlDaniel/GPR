#include <iostream>
#include "GPR.h"

GPR::GPR(std::vector<std::tuple<std::vector<double>, double, double>> a_trainData, double a_sigma_p, double a_l){

	m_trainData = a_trainData;
	m_sigma_p = a_sigma_p;
	m_lenght_scale = a_l;
	//sigma_p, lenght scale
	setCovariance( m_sigma_p, m_lenght_scale);
	cholesky(m_covariance);
	inverse();
	multInverseCovTrain();
	//printData();
	//std::cout << "+++++++++++++++++++++++++++++++++++++++++" << std::endl;
}


GPR::~GPR(){}

std::vector<std::vector<double> > cholesky(std::vector<std::vector<double> > A){

	int n = A.size();
	double sum1 = 0.0;
	double sum2 = 0.0;
	double sum3 = 0.0;
	std::vector<std::vector<double> > l(n, std::vector<double>(n));
	l[0][0] = sqrt(A[0][0]);
	for (int j = 1; j <= n - 1; j++)
		l[j][0] = A[j][0] / l[0][0];
	for (int i = 1; i <= (n - 2); i++)
	{
		for (int k = 0; k <= (i - 1); k++)
			sum1 += pow(l[i][k], 2);
		l[i][i] = sqrt(A[i][i] - sum1);
		for (int j = (i + 1); j <= (n - 1); j++)
		{
			for (int k = 0; k <= (i - 1); k++)
				sum2 += l[j][k] * l[i][k];
			l[j][i] = (A[j][i] - sum2) / l[i][i];
		}
	}
	for (int k = 0; k <= (n - 2); k++)
		sum3 += pow(l[n - 1][k], 2);
	l[n - 1][n - 1] = sqrt(A[n - 1][n - 1] - sum3);

	return l;
}

void GPR::printData(){


	for (int i = 0; i < m_trainData.size(); i++){

		//std::cout << "(";
		std::cout << "std::tuple<std::vector<double>, double, double>({";

		for (int j = 0; j < std::get<0>(m_trainData[i]).size(); j++){

			

			 if (j == std::get<0>(m_trainData[i]).size() - 1){
				 std::cout << std::get<0>(m_trainData[i])[j] ;
			 }
			 else{
				 std::cout << std::get<0>(m_trainData[i])[j] << " , ";
			 }

			

		}

	std::cout << " }, " << std::get<1>(m_trainData[i]) << "," << std::get<2>(m_trainData[i]) << "),"<< std::endl;

		//std::cout << " ) " << std::get<1>(m_trainData[i]) << "  "<<std::get<2>(m_trainData[i]) << std::endl;
	}

	std::cout << std::endl;
}


double sqabs(std::vector<double> s1, std::vector<double> s2){

	double abs = 0;


	for (int i = 0; i < s1.size(); i++){

		abs = abs + (s1[i] - s2[i])*(s1[i] - s2[i]);

	}

	//std::cout << "abs: " << sqrt(abs) << std::endl;
	

	return abs;
}

double GPR::exponentialKernel(std::vector<double> s1, std::vector<double> s2, double sigma, double l){

	double lsqo2 = -l*l*2;

	return sigma * exp((1 / lsqo2) * sqabs(s1, s2));



}

double GPR::rationalKernel(std::vector<double> s1, std::vector<double> s2, double sigma, double l){

	double lsqo2 = l*l*0.25;



		return sigma*sigma* pow( (1 + (1 / lsqo2) * sqabs(s1, s2)), -2);
}

void GPR::setCovariance(double sigma_p, double l){

	for (int m = 0; m < m_trainData.size(); m++){

		std::vector<double> tmp;
		for (int n = 0; n < m_trainData.size(); n++){

		
			if (m == n){

				
				tmp.push_back((exponentialKernel(std::get<0>(m_trainData[m]), std::get<0>(m_trainData[n]), sigma_p, l) + std::get<2>(m_trainData[n])));
			
			}else{

				tmp.push_back(exponentialKernel(std::get<0>(m_trainData[m]), std::get<0>(m_trainData[n]), sigma_p, l));
			}

			
		}

		m_covariance.push_back(tmp);
	}

}


void GPR::printCovariance(){

	if (m_covariance.empty()){
		std::cout << "Convariance Matrix is empty:" << std::endl;
		return;
	}


	for (int m = 0; m < m_covariance.size(); m++){

		std::cout << "(";
		for (int n = 0; n < m_covariance.size(); n++){
		
			std::cout << m_covariance[m][n] << " , ";
		}
	
		std::cout << " ) " << std::endl;
	
	}

	std::cout << std::endl;
}





void GPR::cholesky(std::vector<std::vector<double> > a) {
	
	int dim = a.size();

	m_choleskyFactor.resize(dim);

	for (int m = 0; m < dim; m++){
		m_choleskyFactor[m].resize(dim);
	}

	for (int i = 0; i < dim; i++){
		for (int j = 0; j < i + 1; j++){
			double s = 0;
			for (int k = 0; k < j; k++) s += m_choleskyFactor[i][k] * m_choleskyFactor[j][k];

			if (i == j){

				if (a[i][i] - s <= 0){
					std::cout << "Cholesky failed" << std::endl;
					return;
				}
				m_choleskyFactor[i][j] = sqrt(a[i][i] - s);
			}
			else{
				m_choleskyFactor[i][j] = 1.0 / m_choleskyFactor[j][j] * (a[i][j] - s);
			}
		}
	}

	
}

void GPR::printCholesky(){

	for (int i = 0; i < m_choleskyFactor.size(); i++){

		for (int j = 0; j < m_choleskyFactor.size(); j++){
			std::cout << m_choleskyFactor[i][j] << " ";
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;
}


void GPR::inverse() {
	
	int dim = m_choleskyFactor.size();

	m_inverse.resize(dim);

	for (int m = 0; m < dim; m++){
		m_inverse[m].resize(dim);
	}
	for (int i = 0; i < dim; i++)
		for (int j = 0; j <= i; j++){

		double sum = (i == j ? 1. : 0.);
		for (int k = i - 1; k >= j; k--) sum -= m_choleskyFactor[i][k] * m_inverse[j][k];
		m_inverse[j][i] = sum / m_choleskyFactor[i][i];
	}

		for (int i = dim - 1; i >= 0; i--)
		for (int j = 0; j <= i; j++){
		
			double sum = (i < j ? 0. : m_inverse[j][i]);
			for (int k = i + 1; k < dim; k++) sum -= m_choleskyFactor[k][i] * m_inverse[j][k];
			m_inverse[i][j] = m_inverse[j][i] = sum / m_choleskyFactor[i][i];
	}
}


void GPR::printInverse(){

	for (int i = 0; i < m_inverse.size(); i++){

		for (int j = 0; j < m_inverse.size(); j++){
			std::cout << m_inverse[i][j] << " ";
		}

		std::cout << std::endl;
	}

	std::cout << std::endl;
}

// Cov^-1 * y
void GPR::multInverseCovTrain(){

	for (int i = 0; i < m_inverse.size(); i++){
		double s = 0;
		for (int j = 0; j < m_inverse.size(); j++){
		
			s = s + m_inverse[i][j] * std::get<1>(m_trainData[j]);
		
		}
		m_interimResult.push_back(s);
	}
}

void GPR::printInterimResult(){

	for (int i = 0; i < m_interimResult.size(); i++){

		std::cout << m_interimResult[i] << std::endl;
	}

	std::cout << std::endl;
}

double GPR::dot(std::vector<double> a, std::vector<double> b){
	double s = 0;

	for (int i = 0; i < a.size() ; i++){

		s = s + a[i] * b[i];
		
	}
	
	return s;
}

double GPR::getMean(std::vector<double>  s){

	std::vector<double>  k;
	
	for (int i = 0; i < m_trainData.size(); i++){
		k.push_back(exponentialKernel(std::get<0>(m_trainData[i]), s, m_sigma_p, m_lenght_scale));
		
	}

	

	return dot(k, m_interimResult);
}





double GPR::getVariance(std::vector<double>  s){

	std::vector<double>  k;

	for (int i = 0; i < m_trainData.size(); i++){
		k.push_back(exponentialKernel(std::get<0>(m_trainData[i]), s, m_sigma_p, m_lenght_scale));

	}


	// k^T * Cov^-1 * k
	double result = 0;
	
	for (int i = 0; i < m_inverse.size(); i++){
		double tmp = 0;
		for (int j = 0; j < m_inverse.size(); j++){

			tmp = tmp + m_inverse[i][j] * k[j];

		}
		result = result + k[i] * tmp;
	}
	

	return exponentialKernel(s, s, m_sigma_p, m_lenght_scale) - result;
}