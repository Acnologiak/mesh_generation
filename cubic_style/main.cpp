#include <iostream>

#include "mesh_generation.h"

int main()
{
	figure fig;
	mesh_generation vova;
	glm::ivec3 p1(0, 0, 0), p2(128, 128, 128);

	vova.m_gen(fig, p1, p2, 12);
	vova.m_gen(fig, p1, p2, 12);
	vova.m_gen(fig, p1, p2, 12);
	vova.m_gen(fig, p1, p2, 12);
	vova.m_gen(fig, p1, p2, 12);


	system("pause");
	return 0;
}