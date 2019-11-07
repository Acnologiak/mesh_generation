#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <mutex>
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
	mesh_generation(glm::ivec3 _size_block);
	~mesh_generation();

	void gen_mesh(figure *fig, glm::ivec3 _side);
	void gen_mesh_thr(figure* fig, glm::ivec3 _side, glm::ivec3 _size, int _n_thr);
	void create_matrix();
	void delete_matrix();
private:
	glm::ivec3 size_block;
	std::atomic_int compl_thr = 0;
	std::atomic_int n_points = 0;
	std::mutex lock_fig;

	point_matrix*** matrix_3d;
	figure* base_fig;

	void func(figure* fig, glm::ivec3 _side);
	void check_points(figure* fig, glm::ivec3 _side);
	void check_polygons(figure* fig);
	void func_thr(glm::ivec3 _size_block, glm::ivec3 _side, glm::ivec3 _size, int _n_thr, int _x);
};

/*
Даний код працює зі сторонами які кратні 32 коректно 
*/