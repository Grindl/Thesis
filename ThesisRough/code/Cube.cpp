#include "Cube.hpp"

CubeMaker::CubeMaker()
{

}

CubeMaker::CubeMaker(const float* origin, const float* farCorner)
{

	verticiesFromOriginAndFarCorner(origin, farCorner);
}

const void CubeMaker::render()
{
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glBegin(GL_QUADS);
	{
		//far 2367
		glColor4f(0.5f, 0.0f, 0.5f, 0.5f);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);


		//top 1357
		glColor4f(0.5f, 0.5f, 1.f, 0.5f);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);

		//bottom 0246
		glColor4f(0.5f, 0.5f, 0.0f, 0.5f);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);

		//left 0123
		glColor4f(0.0f, 0.5f, 0.5f, 0.5f);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);


		//right 4567
		glColor4f(1.0f, 0.5f, 0.5f, 0.5f);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);


		//front 0145
		glColor4f(0.5f, 1.f, 0.5f, 0.5f);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);

	}
	glEnd();
}

/*const void CubeMaker::render(CUBE_MATERIAL cubeType, unsigned char sideFlags)
{
	float lightValue = 1.f;
	if(sideFlags & 1)
	{
		//far 2367
		lightValue = 0.7f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);

	}
	if(sideFlags & 2)
	{
		//top 1357
		lightValue = 1.f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);
	}
	if(sideFlags & 4)
	{
		//bottom 0246
		lightValue = 0.5f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);
	}
	if(sideFlags & 8)
	{
		//left 0123
		lightValue = 0.9f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[2][0], verteces[2][1], verteces[2][2]);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);
		glVertex3f(verteces[3][0], verteces[3][1], verteces[3][2]);

	}
	if(sideFlags & 16)
	{
		//right 4567
		lightValue = 0.6f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[6][0], verteces[6][1], verteces[6][2]);
		glVertex3f(verteces[7][0], verteces[7][1], verteces[7][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);
	}
	if(sideFlags & 32)
	{
		//front 0145
		lightValue = 0.8f;
		glColor3f(CUBE_TABLE[cubeType].color[0]*lightValue, CUBE_TABLE[cubeType].color[1]*lightValue, CUBE_TABLE[cubeType].color[2]*lightValue);
		glVertex3f(verteces[0][0], verteces[0][1], verteces[0][2]);
		glVertex3f(verteces[4][0], verteces[4][1], verteces[4][2]);
		glVertex3f(verteces[5][0], verteces[5][1], verteces[5][2]);
		glVertex3f(verteces[1][0], verteces[1][1], verteces[1][2]);
	}
}*/

void CubeMaker::move(const float* translation)
{
	for(int i = 0; i < 8; i++)
	{
		verteces[i][0] +=translation[0];
		verteces[i][1] +=translation[1];
		verteces[i][2] +=translation[2];
	}
}

void CubeMaker::setPosition(const float* position)
{
	float diagonal[3];
	diagonal[0] = verteces[7][0] - verteces[0][0];
	diagonal[1] = verteces[7][1] - verteces[0][1];
	diagonal[2] = verteces[7][2] - verteces[0][2];
	float farCorner[3];
	farCorner[0] = position[0]+diagonal[0];
	farCorner[1] = position[1]+diagonal[1];
	farCorner[2] = position[2]+diagonal[2];
	verticiesFromOriginAndFarCorner(position, farCorner);
}

void CubeMaker::verticiesFromOriginAndFarCorner(const float* origin, const float* farCorner)
{
	//bottom left front
	verteces[0][0] = origin[0];
	verteces[0][1] = origin[1];
	verteces[0][2] = origin[2];

	//top left front
	verteces[1][0] = origin[0];
	verteces[1][1] = origin[1];
	verteces[1][2] = farCorner[2];

	//bottom left back
	verteces[2][0] = origin[0];
	verteces[2][1] = farCorner[1];
	verteces[2][2] = origin[2];

	//top left back
	verteces[3][0] = origin[0];
	verteces[3][1] = farCorner[1];
	verteces[3][2] = farCorner[2];

	//bottom right front
	verteces[4][0] = farCorner[0];
	verteces[4][1] = origin[1];
	verteces[4][2] = origin[2];

	//top right front
	verteces[5][0] = farCorner[0];
	verteces[5][1] = origin[1];
	verteces[5][2] = farCorner[2];

	//bottom right back
	verteces[6][0] = farCorner[0];
	verteces[6][1] = farCorner[1];
	verteces[6][2] = origin[2];

	//top right back
	verteces[7][0] = farCorner[0];
	verteces[7][1] = farCorner[1];
	verteces[7][2] = farCorner[2];
}

void CubeMaker::verticiesFromOriginAndFarCorner(const int* origin, const int* farCorner)
{
	float o2[3];
	float f2[3];
	o2[0] = (float)origin[0];
	o2[1] = (float)origin[1];
	o2[2] = (float)origin[2];
	f2[0] = (float)farCorner[0];
	f2[1] = (float)farCorner[1];
	f2[2] = (float)farCorner[2];
	verticiesFromOriginAndFarCorner(o2, f2);
}
