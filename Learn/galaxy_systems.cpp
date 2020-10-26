#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <random>
#include "shader_class.h"
#include "galaxy_systems.h"
#include "stb_image.h"

void TimeStepTicker::TickStarTimeStep(Star& star, int pertN, double pertAmp)
{
	//写崩了没办法
	return;
	star.m_pos = OrbitCalculator::Compute(star.m_angle, star.m_a, star.m_b, star.m_theta, star.m_center, pertN, pertAmp);	
}

void TimeStepTicker::TickGalaxyTimeStep(Galaxy& gal, double time)
{
	gal.m_timeStep = time;
	gal.m_time += time;
	/*
	对不起天文学家们，对不起亚里士多德，对不起伽利略，对不起开普勒，对不起拉格朗日
	for (int i = 0; i < gal.m_numStars; ++i)
	{
		gal.m_pStars[i].m_theta += (gal.m_pStars[i].m_velTheta * time);
		TickStarTimeStep(gal.m_pStars[i], gal.m_pertN, gal.m_pertAmp);
	}
	for (int i = 0; i < gal.m_numDust; ++i)
	{
		gal.m_pDust[i].m_theta += (gal.m_pDust[i].m_velTheta * time);
		TickStarTimeStep(gal.m_pDust[i], gal.m_pertN, gal.m_pertAmp);
	}
	for (int i = 0; i < gal.m_numH2 * 2; ++i)
	{
		gal.m_pH2[i].m_theta += (gal.m_pH2[i].m_velTheta * time);
		TickStarTimeStep(gal.m_pH2[i], gal.m_pertN, gal.m_pertAmp);
	}
	*/
	for (int i = 0; i < gal.m_numStars; ++i)
	{
		float x = gal.m_pStars[i].m_pos.x;
		float y = gal.m_pStars[i].m_pos.y;
		float dist = sqrtf(x * x + y * y);
		float arccos = std::acosf(x / dist);
		arccos = y > 0 ? arccos : -arccos;
		arccos += gal.m_pStars[i].m_biasVel;
		gal.m_pStars[i].m_pos = glm::vec2(dist * cosf(arccos), dist * sinf(arccos));
	}
	for (int i = 0; i < gal.m_numDust; ++i)
	{
		gal.m_pDust[i].m_biasVel = 
			gal.m_pDust[i].m_biasCore 
			* (0.7 + 0.3 
			* sin(gal.m_time + gal.m_pDust[i].m_biasTime));
	}
}

void GalaxyInitializer::InitializeGalaxy(Galaxy& gal, double sigma)
{
	delete[] gal.m_pDust;
	gal.m_pDust = new Star[gal.m_numDust];
	delete[] gal.m_pStars;
	gal.m_pStars = new Star[gal.m_numStars];
	delete[] gal.m_pH2;
	gal.m_pH2 = new Star[gal.m_numH2 * 2];

	// The first three stars can be used for aligning the
	// camera with the galaxy rotation.
	// First star ist the black hole at the centre
	gal.m_pStars[0].m_a = 0;
	gal.m_pStars[0].m_b = 0;
	gal.m_pStars[0].m_angle = 0;
	gal.m_pStars[0].m_theta = 0;
	gal.m_pStars[0].m_velTheta = 0;
	gal.m_pStars[0].m_center = glm::vec2(0, 0);
	gal.m_pStars[0].m_velTheta = gal.GetOrbitalVelocity((gal.m_pStars[0].m_a + gal.m_pStars[0].m_b) / 2.0);
	gal.m_pStars[0].m_temp = 6000;
	// second star is at the edge of the core area
	gal.m_pStars[1].m_a = gal.m_radCore;
	gal.m_pStars[1].m_b = gal.m_radCore * gal.GetExcentricity(gal.m_radCore);
	gal.m_pStars[1].m_angle = gal.GetAngularOffset(gal.m_radCore);
	gal.m_pStars[1].m_theta = 0;
	gal.m_pStars[1].m_center = glm::vec2(0, 0);
	gal.m_pStars[1].m_velTheta = gal.GetOrbitalVelocity((gal.m_pStars[1].m_a + gal.m_pStars[1].m_b) / 2.0);
	gal.m_pStars[1].m_temp = 6000;
	// third star is at the edge of the disk
	gal.m_pStars[2].m_a = gal.m_radGalaxy;
	gal.m_pStars[2].m_b = gal.m_radGalaxy * gal.GetExcentricity(gal.m_radGalaxy);
	gal.m_pStars[2].m_angle = gal.GetAngularOffset(gal.m_radGalaxy);
	gal.m_pStars[2].m_theta = 0;
	gal.m_pStars[2].m_center = glm::vec2(0, 0);
	gal.m_pStars[2].m_velTheta = gal.GetOrbitalVelocity((gal.m_pStars[2].m_a + gal.m_pStars[2].m_b) / 2.0);
	gal.m_pStars[2].m_temp = 6000;

	// cell width of the histogramm
	double dh = (double)gal.m_radFarField / 100.0;
	// Initialize the stars
	CumulativeDistributionFunction cdf;
	cdf.SetupRealistic(1.0,		// Maximalintensität
		0.02,					// k (bulge)
		gal.m_radGalaxy / 3.0,	// disc skalenlänge
		gal.m_radCore,			// bulge radius
		0,						// start der intensitätskurve
		gal.m_radFarField,		// ende der intensitätskurve
		1000);					// Anzahl der stützstellen
	for (int i = 3; i < gal.m_numStars; ++i)
	{
		double rad = cdf.ValFromProb((double)rand() / (double)RAND_MAX);
		gal.m_pStars[i].m_a = rad;
		gal.m_pStars[i].m_b = rad * gal.GetExcentricity(rad);
		gal.m_pStars[i].m_angle = gal.GetAngularOffset(rad);
		gal.m_pStars[i].m_theta = 360.0 * ((double)rand() / (double)RAND_MAX);
		gal.m_pStars[i].m_velTheta = gal.GetOrbitalVelocity(rad);
		gal.m_pStars[i].m_center = glm::vec2(0, 0);
		gal.m_pStars[i].m_temp = 6000 + (20000 * ((double)rand() / (double)RAND_MAX)) - 2000;
		gal.m_pStars[i].m_mag = 0.3 + 0.2 * (double)rand() / (double)RAND_MAX;
		int idx = (int)std::min(1.0 / dh * (gal.m_pStars[i].m_a + gal.m_pStars[i].m_b) / 2.0, 99.0);
	}

	// Initialise Dust
	double x, y, rad;
	for (int i = 0; i < gal.m_numDust; ++i)
	{
		if (i % 4 == 0)
		{
			rad = cdf.ValFromProb((double)rand() / (double)RAND_MAX);
		}
		else
		{
			x = 2 * gal.m_radGalaxy * ((double)rand() / (double)RAND_MAX) - gal.m_radGalaxy;
			y = 2 * gal.m_radGalaxy * ((double)rand() / (double)RAND_MAX) - gal.m_radGalaxy;
			rad = sqrt(x*x + y * y);
		}

		gal.m_pDust[i].m_a = rad;
		gal.m_pDust[i].m_b = rad * gal.GetExcentricity(rad);
		gal.m_pDust[i].m_angle = gal.GetAngularOffset(rad);
		gal.m_pDust[i].m_theta = 360.0 * ((double)rand() / (double)RAND_MAX);
		gal.m_pDust[i].m_velTheta = gal.GetOrbitalVelocity((gal.m_pDust[i].m_a + gal.m_pDust[i].m_b) / 2.0);
		gal.m_pDust[i].m_center = glm::vec2(0, 0);

		// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
		// the following temperature distribution (no science here it just looks right)
		gal.m_pDust[i].m_temp = 5000 + rad / 4.5;

		gal.m_pDust[i].m_mag = 0.015 + 0.01 * (double)rand() / (double)RAND_MAX;
		int idx = (int)std::min(1.0 / dh * (gal.m_pDust[i].m_a + gal.m_pDust[i].m_b) / 2.0, 99.0);
	}

	// Initialise H2
	for (int i = 0; i < gal.m_numH2; ++i)
	{
		x = 2 * (gal.m_radGalaxy) * ((double)rand() / (double)RAND_MAX) - (gal.m_radGalaxy);
		y = 2 * (gal.m_radGalaxy) * ((double)rand() / (double)RAND_MAX) - (gal.m_radGalaxy);
		rad = sqrt(x * x + y * y);

		int k1 = 2 * i;
		gal.m_pH2[k1].m_a = rad;
		gal.m_pH2[k1].m_b = rad * gal.GetExcentricity(rad);
		gal.m_pH2[k1].m_angle = gal.GetAngularOffset(rad);
		gal.m_pH2[k1].m_theta = 360.0 * ((double)rand() / (double)RAND_MAX);
		gal.m_pH2[k1].m_velTheta = gal.GetOrbitalVelocity((gal.m_pH2[k1].m_a + gal.m_pH2[k1].m_b) / 2.0);
		gal.m_pH2[k1].m_center = glm::vec2(0, 0);
		gal.m_pH2[k1].m_temp = 6000 + (6000 * ((double)rand() / (double)RAND_MAX)) - 3000;
		gal.m_pH2[k1].m_mag = 0.1 + 0.05 * (double)rand() / (double)RAND_MAX;
		int idx = (int)std::min(1.0 / dh * (gal.m_pH2[k1].m_a + gal.m_pH2[k1].m_b) / 2.0, 99.0);

		// Create second point 100 pc away from the first one
		int dist = 1000;
		int k2 = 2 * i + 1;
		gal.m_pH2[k2].m_a = (rad + dist);
		gal.m_pH2[k2].m_b = (rad)* gal.GetExcentricity(rad);
		gal.m_pH2[k2].m_angle = gal.GetAngularOffset(rad);
		gal.m_pH2[k2].m_theta = gal.m_pH2[k1].m_theta;
		gal.m_pH2[k2].m_velTheta = gal.m_pH2[k1].m_velTheta;
		gal.m_pH2[k2].m_center = gal.m_pH2[k1].m_center;
		gal.m_pH2[k2].m_temp = gal.m_pH2[k1].m_temp;
		gal.m_pH2[k2].m_mag = gal.m_pH2[k1].m_mag;
		idx = (int)std::min(1.0 / dh * (gal.m_pH2[k2].m_a + gal.m_pH2[k2].m_b) / 2.0, 99.0);
	}
	std::default_random_engine e;
	std::uniform_real_distribution<float> us(-1.0, 1.0);
	std::uniform_real_distribution<float> ue(0.0, 1.0);
	std::uniform_real_distribution<float> u1(0.0, 0.2);
	std::uniform_real_distribution<float> u2(0.2, 0.6);
	std::uniform_real_distribution<float> u3(0.6, 0.9);
	std::uniform_real_distribution<float> u4(0.9, 1.2);
	std::uniform_real_distribution<float> u5(1.2, 1.41);
	std::uniform_real_distribution<float> ud1(0.1, 0.3);
	std::uniform_real_distribution<float> ud2(0.3, 0.7);
	std::uniform_real_distribution<float> ud3(0.7, 1.4); 
	std::uniform_real_distribution<float> udb1(0.7, 1.5);
	std::uniform_real_distribution<float> udb2(1.5, 3);
	std::uniform_real_distribution<float> ut(0, 6.28);
	std::default_random_engine ed;
	/*
	for (int i = 0; i < gal.m_numStars; ++i)
	{
		gal.m_pStars[i].m_pos = glm::vec2(u(e), u(e));
	}
	for (int i = 0; i < gal.m_numH2; ++i)
	{
		gal.m_pH2[i].m_pos = glm::vec2(u(e), u(e));
	}
	for (int i = 0; i < gal.m_numDust; ++i)
	{
		gal.m_pDust[i].m_pos = glm::vec2(u(e), u(e));
	}*/
	for (int i = 0; i < gal.m_numStars; ++i)
	{
		float exp = ue(e);
		float bias = us(e) * 3.14159265358;
		float dist = 
			exp < 0.92 ? 
			(exp < 0.72 ?
			(exp < 0.49 ?
			(exp < 0.22 ? 
			u1(e) : u2(e))
			:u3(e)) 
			:u4(e))
			:u5(e);
		float x = dist * cosf(bias);
		float y = dist * sinf(bias);
		exp = ue(e);
		float cutoff = 0.3;
		float cutoffVel = 0.0003;
		float vel = dist < cutoff ? cutoffVel * (dist / cutoff) : cutoffVel;
		vel = dist > 1.0 ? 0 : vel;
		gal.m_pStars[i].m_biasVel = vel / dist;
		gal.m_pStars[i].m_pos = glm::vec2(x, y);
	}
	for (int i = 0; i < gal.m_numH2; ++i)
	{
		float bias = us(e) * 3.14159265358;
		float dist = ue(e);
		float x = dist * cosf(bias);
		float y = dist * sinf(bias);
		gal.m_pH2[i].m_pos = glm::vec2(x, y);
	}
	for (int i = 0; i < gal.m_numDust; ++i)
	{

		float bias = us(ed) * 3.14159265358;
		float exp = ue(e);
		float dist =
			exp < 0.5 ?
			(exp < 0.10 ?
			ud1(e) : ud2(e))
			: ud3(e);
		float x = dist * cosf(bias);
		float y = dist * sinf(bias);
		exp = ue(e);
		float cutoff = 0.7;
		gal.m_pDust[i].m_biasTime = ut(ed);
		gal.m_pDust[i].m_biasCore =
			(exp < 0.7 ? udb1(e) : udb2(e)) * (1 - 0.5 * (dist - cutoff));
		gal.m_pDust[i].m_biasVel = gal.m_pDust[i].m_biasCore;
		gal.m_pDust[i].m_pos = glm::vec2(x, y);
	}

}

GalaxyContainer::GalaxyContainer()
{
	m_initializer = new GalaxyInitializer();
	m_stepticker = new TimeStepTicker();
	m_renderer = new GalaxyRenderer();
	m_galaxy = new Galaxy();
	m_initializer->InitializeGalaxy(*m_galaxy);
}

void GalaxyContainer::Tick(double dt)
{
	// Render Galaxy
	m_renderer->RenderGalaxy(*m_galaxy);
	// Tick
	m_stepticker->TickGalaxyTimeStep(*m_galaxy, dt);
}

unsigned int GalaxyRenderer::loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

GalaxyRenderer::GalaxyRenderer() :
	m_texture(loadTexture("particle.bmp")),
	m_basicTexture(loadTexture("basic.png")),
	m_shader(new Shader("star.vs", "star.fs")),
	m_basic(new Shader("basic.vs", "basic.fs")),
	m_dustShader(new Shader("dust.vs", "dust.fs")),
	VAO(0), VBO(0)
{
	m_colorfunction = new KervinColorFunction();
	glEnable(GL_PROGRAM_POINT_SIZE);
}

void GalaxyRenderer::RenderGalaxy(const Galaxy& galaxy)
{
	/*
	以下这段是用来逗你的代码
	Shader testShader("star.vs", "star.fs");
	unsigned int VAO, VBO;
	float vertices[] = {
		0.0, 0.0, 1.0f, 1.0f, 1.0f
	};
	glEnable(GL_PROGRAM_POINT_SIZE);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_starTexture);
	testShader.use();
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, 1);
	*/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawDust(galaxy);
	DrawH2(galaxy);
	DrawStar(galaxy);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	float vertices[] = {
		// positions  // texture coords
		 1.0f,  1.0f, 1.0, 0.0f,   // top right
		 1.0f, -1.0f, 1.0f, 1.0f,   // bottom right
		-1.0f, -1.0f, 0.0f, 1.0f,   // bottom left
		-1.0f,  1.0f, 0.0f, 0.0f    // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_basicTexture);
	m_basic->use();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

//TODO & 签名式可能要改
void GalaxyRenderer::DrawStar(const Galaxy& galaxy)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	float vertices[100000];
	const Star* pstar = galaxy.m_pStars;
	int num = galaxy.m_numStars;
	int count = 0;
	// preset vertices 
	for (int i = 0; i < num; ++i)
	{
		glm::vec2 pos = pstar[i].m_pos;
		glm::vec3 color = m_colorfunction->RGBFromKervin(pstar[i].m_temp);
		vertices[count * 5] = pos.x / 1.57;
		vertices[count * 5 + 1] = pos.y;
		vertices[count * 5 + 2] = color.r;
		vertices[count * 5 + 3] = color.g;
		vertices[count * 5 + 4] = color.b;
		count += 1;
	}
	//TODO? is that right?
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	m_shader->use();
	m_shader->setFloat("PointSize", 4);
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, num);
	
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	count = 0;
	// preset vertices 
	for (int i = 1; i < num / 30; ++i)
	{
		glm::vec2 pos = pstar[i].m_pos;
		glm::vec3 color = m_colorfunction->RGBFromKervin(pstar[i].m_temp);
		vertices[count * 5] = pos.x / 1.57;
		vertices[count * 5 + 1] = pos.y;
		vertices[count * 5 + 2] = color.r * 0.6;
		vertices[count * 5 + 3] = color.g * 0.6;
		vertices[count * 5 + 4] = color.b * 0.6;
		count += 1;
	}
	//TODO? is that right?
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	m_shader->use();
	m_shader->setFloat("PointSize", 3);
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, num);
}

void GalaxyRenderer::DrawDust(const Galaxy& galaxy)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	float vertices[100000];
	const Star* pstar = galaxy.m_pDust;
	int num = galaxy.m_numDust;
	int count = 0;
	// preset vertices 
	for (int i = 0; i < num; ++i)
	{
		glm::vec2 pos = pstar[i].m_pos;
		glm::vec3 color = m_colorfunction->RGBFromKervin(pstar[i].m_temp);
		vertices[count * 6] = pos.x / 1.57;
		vertices[count * 6 + 1] = pos.y;
		vertices[count * 6 + 2] = pstar[i].m_biasVel;
		vertices[count * 6 + 3] = color.r * 0.8;
		vertices[count * 6 + 4] = color.g * 0.2;
		vertices[count * 6 + 5] = color.b * 0.4;
		count += 1;
	}
	//TODO? is that right?
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	m_dustShader->use();
	m_dustShader->setFloat("PointSize", 96);
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, num);
}

//TODO
void GalaxyRenderer::DrawH2(const Galaxy& galaxy)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	float vertices[40000];
	const Star* pstar = galaxy.m_pH2;
	int num = galaxy.m_numH2;
	int count = 0;
	// preset vertices 
	for (int i = 0; i < num; ++i)
	{
		glm::vec2 pos = pstar[i].m_pos;
		glm::vec3 color = m_colorfunction->RGBFromKervin(pstar[i].m_temp);
		vertices[count * 5] = pos.x / 1.57;
		vertices[count * 5 + 1] = pos.y;
		vertices[count * 5 + 2] = color.r;
		vertices[count * 5 + 3] = color.g;
		vertices[count * 5 + 4] = color.b;
		count += 1;
	}
	//TODO? is that right?
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	m_shader->use();
	m_shader->setFloat("PointSize", 32);
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, num);
}
