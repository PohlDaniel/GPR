#ifndef _GPR_H
#define _GPR_H

#include <vector>
#include <tuple>

#include "Vector.h"
#include "GPR.h"
#include "Interpolation.h"

class GPR : public Interpolation{

public:

	GPR();
	GPR(std::vector<std::tuple<std::vector<double>, double, double>> trainData, double sigma_p, double l);
	~GPR();

	void printData();
	void printCovariance();
	void setCovariance(double sigma_p, double l);

	
	void printCholesky();
	void printInverse();
	void printInterimResult();

	void cholesky(std::vector<std::vector<double> > a);
	void inverse();

	double getMean(std::vector<double>  s);
	double getVariance(std::vector<double>  s);



	double exponentialKernel(std::vector<double> s1, std::vector<double> s2, double sigma, double l);
	
	double rationalKernel(std::vector<double> s1, std::vector<double> s2, double sigma_b, double sigma_v);



	void multInverseCovTrain();
	double dot(std::vector<double> a, std::vector<double> b);

	std::vector<std::tuple<std::vector<double>, double, double>> m_trainData;
	std::vector<std::vector<double> > m_covariance;
	std::vector<std::vector<double> > m_choleskyFactor;
	std::vector<std::vector<double> > m_inverse;
	std::vector<double> m_interimResult;
	double m_sigma_p, m_lenght_scale;
};

#endif