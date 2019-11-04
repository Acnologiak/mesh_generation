#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include "glm/glm.hpp"



struct figure
{
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> v_normal;
	std::vector<glm::ivec3> polygons;
};


struct point_matrix
{
	int number = 0;
	bool point_position = false;
	bool point_foreign = false;
};


class mesh_generation
{
public:
	mesh_generation();
	~mesh_generation();

	//генерація меша
	bool m_gen(figure &fig, glm::ivec3 p1, glm::ivec3 p2, int _number_thr);
private:
	glm::ivec3 size;
	glm::ivec3 side;

	std::atomic_int status = 0;
	std::atomic_int number_thr_complete = 0;

	point_matrix ***matrix_3d;

	void pause_thr_func(int _number_thr);
	//функція потоку
	void thr_func(int _number_thr, int x);
	//функція генерації
	void func(int _number_thr, int x);
	//створення матриці
	void create_matrix();
	//видалення матриці
	void delete_matrix();
};

