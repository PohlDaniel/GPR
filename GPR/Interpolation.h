#ifndef _INTERPOLATION_H
#define _INTERPOLATION_H

#include <vector>

class Interpolation{

public:

	Interpolation();
	~Interpolation();

	virtual double getMean(std::vector<double>  s) = 0;
	virtual double getVariance(std::vector<double>  s) = 0;


	std::vector<double> m_means;
	std::vector<double> m_variances;
};

////////////////////////////////////////////////////////////////////////////////////////////




#endif