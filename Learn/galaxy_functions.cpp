#include "galaxy_functions.h"
#include "galaxy_defines.h"
#include "specrend.h"

#include <cmath>
#include <algorithm>

glm::vec2 OrbitCalculator::Compute(double angle, double a, double b, double theta, const glm::vec2 &p, int pertN, double pertAmp)
{
	double beta = -angle,
		alpha = theta * DEG_TO_RAD;

	// temporaries to save cpu time
	double  cosalpha = cos(alpha),
		sinalpha = sin(alpha),
		cosbeta = cos(beta),
		sinbeta = sin(beta);

	glm::vec2 pos = glm::vec2(p.x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),
		p.y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));

	// Add small perturbations to create more spiral arms
	if (pertAmp > 0 && pertN > 0)
	{
		pos.x += (a / pertAmp) * sin(alpha * 2 * pertN);
		pos.y += (a / pertAmp) * cos(alpha * 2 * pertN);
	}
	return pos;
}

CumulativeDistributionFunction::CumulativeDistributionFunction()
	:m_pDistFun(nullptr)
	, m_vM1()
	, m_vY1()
	, m_vX1()
	, m_vM2()
	, m_vY2()
	, m_vX2()
{}

void CumulativeDistributionFunction::SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps)
{
	m_fMin = min;
	m_fMax = max;
	m_nSteps = nSteps;
	m_I0 = I0;
	m_k = k;
	m_a = a;
	m_RBulge = RBulge;
	m_pDistFun = &CumulativeDistributionFunction::Intensity;
	// build the distribution function
	BuildCDF(m_nSteps);
}

void CumulativeDistributionFunction::BuildCDF(int nSteps)
{
	double h = (m_fMax - m_fMin) / nSteps;
	double x = 0, y = 0;

	m_vX1.clear();
	m_vY1.clear();
	m_vX2.clear();
	m_vY2.clear();
	m_vM1.clear();
	m_vM2.clear();

	// Simpson rule for integration of the distribution function
	m_vY1.push_back(0.0);
	m_vX1.push_back(0.0);
	for (int i = 0; i < nSteps; i += 2)
	{
		x = (i + 2) *h;
		y += h / 3 * ((this->*m_pDistFun)(m_fMin + i * h) + 4 * (this->*m_pDistFun)(m_fMin + (i + 1)*h) + (this->*m_pDistFun)(m_fMin + (i + 2)*h));

		m_vM1.push_back((y - m_vY1.back()) / (2 * h));
		m_vX1.push_back(x);
		m_vY1.push_back(y);

		//    printf("%2.2f, %2.2f, %2.2f\n", m_fMin + (i+2) * h, v, h);
	}
	m_vM1.push_back(0.0);

	// all arrays must have the same length
	if (m_vM1.size() != m_vX1.size() || m_vM1.size() != m_vY1.size())
		throw std::runtime_error("CumulativeDistributionFunction::BuildCDF: array size mismatch (1)!");

	// normieren
	for (std::size_t i = 0; i < m_vY1.size(); ++i)
	{
		m_vY1[i] /= m_vY1.back();
		m_vM1[i] /= m_vY1.back();
	}

	//
	m_vX2.push_back(0.0);
	m_vY2.push_back(0.0);

	double p = 0;
	h = 1.0 / nSteps;
	for (int i = 1, k = 0; i < nSteps; ++i)
	{
		p = (double)i * h;

		for (; m_vY1[k + 1] <= p; ++k)
		{
		}

		y = m_vX1[k] + (p - m_vY1[k]) / m_vM1[k];

		m_vM2.push_back((y - m_vY2.back()) / h);
		m_vX2.push_back(p);
		m_vY2.push_back(y);
	}
	m_vM2.push_back(0.0);

	// all arrays must have the same length
	if (m_vM2.size() != m_vX2.size() || m_vM2.size() != m_vY2.size())
		throw std::runtime_error("CumulativeDistributionFunction::BuildCDF: array size mismatch (1)!");

}

CumulativeDistributionFunction::~CumulativeDistributionFunction()
{}

double CumulativeDistributionFunction::ProbFromVal(double fVal)
{
	if (fVal<m_fMin || fVal>m_fMax)
		throw std::runtime_error("out of range");

	double h = 2 * ((m_fMax - m_fMin) / m_nSteps);
	int i = (int)((fVal - m_fMin) / h);
	double remainder = fVal - i * h;

	assert(i >= 0 && i < (int)m_vM1.size());
	return (m_vY1[i] + m_vM1[i] * remainder) /* / m_vVal.back()*/;
}

double CumulativeDistributionFunction::ValFromProb(double fVal)
{
	if (fVal < 0 || fVal>1)
		throw std::runtime_error("out of range");

	double h = 1.0 / (m_vY2.size() - 1);

	int i = (int)(fVal / h);
	double remainder = fVal - i * h;

	assert(i >= 0 && i < (int)m_vM2.size());
	return (m_vY2[i] + m_vM2[i] * remainder) /* / m_vVal.back()*/;
}

double CumulativeDistributionFunction::IntensityBulge(double R, double I0, double k)
{
	return I0 * exp(-k * pow(R, 0.25));
}

double CumulativeDistributionFunction::IntensityDisc(double R, double I0, double a)
{
	return I0 * exp(-R / a);
}

double CumulativeDistributionFunction::Intensity(double x)
{
	return (x < m_RBulge) ? IntensityBulge(x, m_I0, m_k) : IntensityDisc(x - m_RBulge, IntensityBulge(m_RBulge, m_I0, m_k), m_a);
}

glm::vec3 KervinColorFunction::xyz_to_rgb(struct colourSystem *cs,
	double xc, double yc, double zc)
{
	double xr, yr, zr, xg, yg, zg, xb, yb, zb;
	double xw, yw, zw;
	double rx, ry, rz, gx, gy, gz, bx, by, bz;
	double rw, gw, bw;

	xr = cs->xRed;    yr = cs->yRed;    zr = 1 - (xr + yr);
	xg = cs->xGreen;  yg = cs->yGreen;  zg = 1 - (xg + yg);
	xb = cs->xBlue;   yb = cs->yBlue;   zb = 1 - (xb + yb);

	xw = cs->xWhite;  yw = cs->yWhite;  zw = 1 - (xw + yw);

	/* xyz -> rgb matrix, before scaling to white. */

	rx = (yg * zb) - (yb * zg);  ry = (xb * zg) - (xg * zb);  rz = (xg * yb) - (xb * yg);
	gx = (yb * zr) - (yr * zb);  gy = (xr * zb) - (xb * zr);  gz = (xb * yr) - (xr * yb);
	bx = (yr * zg) - (yg * zr);  by = (xg * zr) - (xr * zg);  bz = (xr * yg) - (xg * yr);

	/* White scaling factors.
	   Dividing by yw scales the white luminance to unity, as conventional. */

	rw = ((rx * xw) + (ry * yw) + (rz * zw)) / yw;
	gw = ((gx * xw) + (gy * yw) + (gz * zw)) / yw;
	bw = ((bx * xw) + (by * yw) + (bz * zw)) / yw;

	/* xyz -> rgb matrix, correctly scaled to white. */

	rx = rx / rw;  ry = ry / rw;  rz = rz / rw;
	gx = gx / gw;  gy = gy / gw;  gz = gz / gw;
	bx = bx / bw;  by = by / bw;  bz = bz / bw;

	/* rgb of the desired point */
	return (glm::vec3(
		(rx * xc) + (ry * yc) + (rz * zc),
		(gx * xc) + (gy * yc) + (gz * zc),
		(bx * xc) + (by * yc) + (bz * zc)
	));
}

KervinColorFunction::KervinColorFunction():
	m_colNum(200), m_t0(1000), m_t1(10000), m_dt((m_t1 - m_t0) / m_colNum)
{
	double x, y, z;
	for (int i = 0; i < m_colNum; ++i)
	{
		glm::vec3 &col = m_col[i];
		colourSystem *cs = &SMPTEsystem;
		bbTemp = m_t0 + m_dt * i;
		spectrum_to_xyz(bb_spectrum, &x, &y, &z);
		col = xyz_to_rgb(cs, x, y, z);
		col = glm::normalize(col);
	}
}

glm::vec3 KervinColorFunction::RGBFromKervin(double temp) const
{
	int idx = (temp - m_t0) / (m_t1 - m_t0) * m_colNum;
	idx = std::min(m_colNum - 1, idx);
	idx = std::max(0, idx);
	return m_col[idx];
}