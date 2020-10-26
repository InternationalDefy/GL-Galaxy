#pragma once
#include "galaxy_components.h"
#include "galaxy_functions.h"
#include "stb_image.h"

class Shader;
class TimeStepTicker {
public:
	TimeStepTicker() {}
	void TickGalaxyTimeStep(Galaxy& gal, double time);
private:
	void TickStarTimeStep(Star& star, int pertN, double pertAmp);
};

class GalaxyInitializer {
public:
	GalaxyInitializer() {}
	void InitializeGalaxy(Galaxy& gal, double sigma = 0.45);
};

class GalaxyRenderer {
private:
	KervinColorFunction* m_colorfunction;
	// draw function
	//签名式可能要改
	void DrawStar(const Galaxy& gal);
	void DrawH2(const Galaxy& gal);
	void DrawDust(const Galaxy& gal);
	unsigned int loadTexture(const char *path);
	unsigned int m_texture;
	unsigned int m_basicTexture;
	unsigned int VAO, VBO;
	Shader* m_shader;
	Shader* m_basic;
	Shader* m_dustShader;
public:
	GalaxyRenderer();
	void RenderGalaxy(const Galaxy& gal);
};

class GalaxyContainer {
private:
	Galaxy* m_galaxy;
	GalaxyInitializer* m_initializer;
	TimeStepTicker* m_stepticker;	
	GalaxyRenderer* m_renderer;
public:
	GalaxyContainer();
	void Tick(double dt);
};