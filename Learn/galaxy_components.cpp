#include "galaxy_components.h"
Galaxy::Galaxy(double rad ,
	double radCore ,
	double deltaAng ,
	double ex1 ,
	double ex2 ,
	double sigma ,
	double velInner,
	double velOuter,
	int numStars ,
	int pertN ,
	double pertAmp ,
	double dustRenderSize)
	: m_elEx1(ex1)
	, m_elEx2(ex2)
	, m_velOrigin(30)
	, m_velInner(velInner)
	, m_velOuter(velOuter)
	, m_angleOffset(deltaAng)
	, m_radCore(radCore)
	, m_radGalaxy(rad)
	, m_radFarField(m_radGalaxy * 2)
	, m_sigma(sigma)
	, m_velAngle(0.000001)
	, m_numStars(numStars)
	, m_numDust(numStars / 128)
	, m_numH2(50)
	, m_pertN(pertN)
	, m_pertAmp(pertAmp)
	, m_time(0)
	, m_timeStep(0)
	, m_bHasDarkMatter(true)
	, m_pos(0, 0)
	, m_pStars(nullptr)
	, m_pDust(nullptr)
	, m_pH2(nullptr)
	, m_dustRenderSize(dustRenderSize)
{
}
Galaxy::~Galaxy()
{
	delete[] m_pStars;
	delete[] m_pDust;
	delete[] m_pH2;
}

double Galaxy::GetExcentricity(double r) const
{
	if (r < m_radCore)
	{
		return 1 + (r / m_radCore) * (m_elEx1 - 1);
	}
	else if (r > m_radCore && r <= m_radGalaxy)
	{
		return m_elEx1 + (r - m_radCore) / (m_radGalaxy - m_radCore) * (m_elEx2 - m_elEx1);
	}
	else if (r > m_radGalaxy && r < m_radFarField)
	{
		return m_elEx2 + (r - m_radGalaxy) / (m_radFarField - m_radGalaxy) * (1 - m_elEx2);
	}
	else
		return 1;
}
double Galaxy::GetAngularOffset(double rad) const
{
	return rad * m_angleOffset;
}
double Galaxy::GetOrbitalVelocity(double rad) const
{
	double vel_kms(0);  // velovity in kilometer per seconds

	// Realistically looking velocity curves for the Wikipedia models.
	struct VelocityCurve
	{
		static double MS(double r)
		{
			double d = 2000;  // Dicke der Scheibe
			double rho_so = 1;  // Dichte im Mittelpunkt
			double rH = 2000; // Radius auf dem die Dichte um die Hälfte gefallen ist
			return rho_so * exp(-r / rH) * (r * r) * M_PI * d;
		}

		static double MH(double r)
		{
			double rho_h0 = 0.15; // Dichte des Halos im Zentrum
			double rC = 2500;     // typische skalenlänge im Halo
			return rho_h0 * 1 / (1 + pow(r / rC, 2)) * (4 * M_PI * pow(r, 3) / 3);
		}

		// Velocity curve with dark matter
		static double v(double r)
		{
			double MZ = 100;
			double G = 6.672e-11;
			return 1280 * sqrt(G *(MH(r) + MS(r) + MZ) / r);
		}

		// velocity curve without dark matter
		static double vd(double r)
		{
			double MZ = 100;
			double G = 6.672e-11;
			return 1280 * sqrt(G *(MS(r) + MZ) / r);
		}
	};

	if (m_bHasDarkMatter)
	{
		//  with dark matter
		vel_kms = VelocityCurve::v(rad);
	}
	else
	{
		// without dark matter:
		vel_kms = VelocityCurve::vd(rad);
	}

	// Calculate velocity in degree per year
	double u = 2 * M_PI * rad * PC_TO_KM;        // Umfang in km
	double time = u / (vel_kms * SEC_PER_YEAR);  // Umlaufzeit in Jahren

	return 360.0 / time;                                   // Grad pro Jahr
}
const glm::vec2& Galaxy::GetStarPos(int idx)
{
	if (idx >= m_numStars)
		throw std::runtime_error("index out of bounds.");

	return m_pStars[idx].m_pos; //GetPos;
}