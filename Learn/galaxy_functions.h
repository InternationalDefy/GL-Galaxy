#pragma once
#include <glm/glm.hpp>
#include <vector>
struct colourSystem;

struct OrbitCalculator 
{
	static glm::vec2 Compute(double angle, double a, double b, double theta, const glm::vec2 &p, int pertN, double pertAmp);
};

class CumulativeDistributionFunction
{
public:
	typedef double (CumulativeDistributionFunction::*dist_fun_t)(double x);
	CumulativeDistributionFunction();
	virtual ~CumulativeDistributionFunction();
	double ProbFromVal(double fVal);
	double ValFromProb(double fVal);
	// function still in initialization stage
	void SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps);
private:
	dist_fun_t m_pDistFun;
	double m_fMin;
	double m_fMax;
	double m_fWidth;
	int m_nSteps;
	// parameters for realistic star distribution
	double m_I0;
	double m_k;
	double m_a;
	double m_RBulge;
	std::vector<double> m_vM1;
	std::vector<double> m_vY1;
	std::vector<double> m_vX1;
	std::vector<double> m_vM2;
	std::vector<double> m_vY2;
	std::vector<double> m_vX2;
	// function still in initialization stage
	void BuildCDF(int nSteps);
	double IntensityBulge(double R, double I0, double k);
	double IntensityDisc(double R, double I0, double a);
	double Intensity(double x);
};

class KervinColorFunction {
private:
	int m_colNum;
	double m_t0, m_t1, m_dt;
	glm::vec3 m_col[200];
	glm::vec3 xyz_to_rgb(struct colourSystem *cs,
		double xc, double yc, double zc);
public:
	glm::vec3 RGBFromKervin(double temp) const;
	KervinColorFunction();
};