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


struct edge
{
	float value;
	int n;
};


class mesh_generation
{
public:
	mesh_generation(glm::ivec3 _size_block);
	~mesh_generation();

	void gen_mesh(figure *fig, glm::ivec3 _side);
	void gen_mesh_thr(figure* fig, glm::ivec3 _side, glm::ivec3 _size, int _n_thr);
	void create_points_matrix();
	void delete_points_matrix();
	void create_edges_matrix();
	void delete_edges_matrix();
private:
	glm::ivec3 size_block;
	std::atomic_int compl_thr = 0;
	std::atomic_int n_points = 0;
	std::mutex lock_fig;

	bool*** matrix_3d;
	edge*** edges_OY;
	edge*** edges_OX;
	edge*** edges_OZ;

	float alpha = 0.1f;

	figure* base_fig;

	void filling_matrix(figure* fig, glm::ivec3 _side);
	void check_points(figure* fig, glm::ivec3 _side);
	void check_polygons(figure* fig);
	void func_thr(glm::ivec3 _size_block, glm::ivec3 _side, glm::ivec3 _size, int _n_thr, int _x);
};


float func(float _x, float _y, float _z);

float m_func_x(float _x1, float _x2, float _y, float _z, float alpha);

float m_func_y(float _x, float _y1, float _y2, float _z, float alpha);

float m_func_z(float _x, float _y, float _z1, float _z2, float alpha);