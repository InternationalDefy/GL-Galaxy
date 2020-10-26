#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include "galaxy_defines.h"
struct Star 
{
	Star()
		:m_theta(0)
		, m_a(0)
		, m_b(0)
		, m_center(0, 0)
	{}
	double m_theta;    // position auf der ellipse
	double m_velTheta; // angular velocity
	double m_angle;    // Schräglage der Ellipse
	double m_a;        // kleine halbachse
	double m_b;        // große halbachse
	double m_temp;     // star temperature
	double m_mag;      // brigtness;
	float m_biasVel;
	float m_biasCore;
	float m_biasTime;
	glm::vec2  m_center;   // center of the elliptical orbit
	glm::vec2  m_pos;      // current position in kartesion koordinates
};

struct Galaxy
{
	// Parameters needed for defining the general structure of the galaxy
	double m_elEx1;          ///< Excentricity of the innermost ellipse
	double m_elEx2;          ///< Excentricity of the outermost ellipse
	double m_velOrigin;      ///< Velovity at the innermost core in km/s
	double m_velInner;       ///< Velocity at the core edge in km/s
	double m_velOuter;       ///< Velocity at the edge of the disk in km/s
	double m_angleOffset;    ///< Angular offset per parsec
	double m_radCore;        ///< Radius of the inner core
	double m_radGalaxy;      ///< Radius of the galaxy
	double m_radFarField;    ///< The radius after which all density waves must have circular shape
	double m_sigma;          ///< Distribution of stars
	double m_velAngle;       ///< Angular velocity of the density waves
	double m_dustRenderSize;
	int m_numStars;          ///< Total number of stars
	int m_numDust;           ///< Number of Dust Particles
	int m_numH2;             ///< Number of H2 Regions
	int m_pertN;
	double m_pertAmp;
	double m_time;
	double m_timeStep;
	bool m_bHasDarkMatter;
	glm::vec2 m_pos;             ///< Center of the galaxy
	Star *m_pStars;          ///< Pointer to an array of star data
	Star *m_pDust;           ///< Pointer to an array of dusty areas
	Star *m_pH2;

	//构造后必须init！
	Galaxy(double rad = 15000,
		double radCore = 6000,
		double deltaAng = 0.019,
		double ex1 = 0.8,
		double ex2 = 1,
		double sigma = 0.45,
		double velInner = 200,
		double velOuter = 300,
		int numStars = 15000,
		int pertN = 0,
		double pertAmp = 0.0,
		double dustRenderSize = 128);
	~Galaxy();
	double GetExcentricity(double r) const;
	double GetAngularOffset(double rad) const;
	double GetOrbitalVelocity(double rad) const;
	const glm::vec2& GetStarPos(int idx);
	//此函数已由galaxy_systems->TimeStepTicker实现
	void SingleTimeStep(double time);
	//此函数待移动至galaxy_systems
	void initStars(double sigma);
};