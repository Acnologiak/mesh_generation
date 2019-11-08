#include "mesh_generation.h"

#include "file_obj.h"
#include "perlin_noise.cpp"

mesh_generation::mesh_generation(glm::ivec3 _size_block)
	: size_block(_size_block)
{
}

mesh_generation::~mesh_generation()
{
}

void mesh_generation::gen_mesh(figure* fig, glm::ivec3 _side)
{
	filling_matrix(fig, _side);
	check_points(fig, _side);
	check_polygons(fig);
}

void mesh_generation::gen_mesh_thr(figure* fig, glm::ivec3 _side, glm::ivec3 _size, int _n_thr)
{
	base_fig = fig;
	for (int i = 0; i < _n_thr; i++)
	{
		std::thread thr(&mesh_generation::func_thr, this, size_block, _side, _size, _n_thr, i);
		thr.detach();
	}
	for (;;)
	{
		if (compl_thr == _n_thr)
		{
			compl_thr = 0;
			n_points = 0;
			std::cout << "The mesh is ready!" << std::endl;
			break;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void mesh_generation::create_points_matrix()
{
	bool** matrix_2d;
	matrix_3d = new bool**[size_block.x];
	for (int i = 0; i < size_block.x; i++)
	{
		matrix_2d = new bool*[size_block.y];
		for (int j = 0; j < size_block.y; j++)
		{
			matrix_2d[j] = new bool[size_block.z];
		}
		matrix_3d[i] = matrix_2d;
	}
}

void mesh_generation::delete_points_matrix()
{
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			delete[] matrix_3d[i][j];
		}
	}
	for (int i = 0; i < size_block.x; i++)
	{
		delete[] matrix_3d[i];
	}
	delete[] matrix_3d;
}

void mesh_generation::create_edges_matrix()
{
	{
		edge** edge_OY_2;
		edges_OY = new edge**[size_block.x];
		for (int i = 0; i < size_block.x; i++)
		{
			edge_OY_2 = new edge*[size_block.y];
			for (int j = 0; j < size_block.y - 1; j++)
			{
				edge_OY_2[j] = new edge[size_block.z];
			}
			edges_OY[i] = edge_OY_2;
		}
	}
	{
		edge** edge_OX_2;
		edges_OX = new edge**[size_block.x];
		for (int i = 0; i < size_block.x - 1; i++)
		{
			edge_OX_2 = new edge*[size_block.y];
			for (int j = 0; j < size_block.y; j++)
			{
				edge_OX_2[j] = new edge[size_block.z];
			}
			edges_OX[i] = edge_OX_2;
		}
	}
	{
		edge** edge_OZ_2;
		edges_OZ = new edge**[size_block.x];
		for (int i = 0; i < size_block.x; i++)
		{
			edge_OZ_2 = new edge*[size_block.y];
			for (int j = 0; j < size_block.y; j++)
			{
				edge_OZ_2[j] = new edge[size_block.z - 1];
			}
			edges_OZ[i] = edge_OZ_2;
		}
	}
}

void mesh_generation::delete_edges_matrix()
{
	{
		for (int i = 0; i < size_block.x; i++)
		{
			for (int j = 0; j < size_block.y - 1; j++)
			{
				delete[] edges_OY[i][j];
			}
		}
		for (int i = 0; i < size_block.x; i++)
		{
			delete[] edges_OY[i];
		}
		delete[] edges_OY;
	}
	{
		for (int i = 0; i < size_block.x - 1; i++)
		{
			for (int j = 0; j < size_block.y; j++)
			{
				delete[] edges_OX[i][j];
			}
		}
		for (int i = 0; i < size_block.x - 1; i++)
		{
			delete[] edges_OX[i];
		}
		delete[] edges_OX;
	}
	{
		for (int i = 0; i < size_block.x; i++)
		{
			for (int j = 0; j < size_block.y; j++)
			{
				delete[] edges_OZ[i][j];
			}
		}
		for (int i = 0; i < size_block.x; i++)
		{
			delete[] edges_OZ[i];
		}
		delete[] edges_OZ;
	}
}

void mesh_generation::filling_matrix(figure* fig, glm::ivec3 _side)
{
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				if (func(i + _side.x, j + _side.y, k + _side.z) < 0)
				{
					matrix_3d[i][j][k] = true;
				}
				else
				{
					matrix_3d[i][j][k] = false;
				}
			}
		}
	}
}

void mesh_generation::check_points(figure* fig, glm::ivec3 _side)
{
	size_t n = 1;
	//OY
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y - 1; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				if (matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k])
				{
					edges_OY[i][j][k].value = m_func_y(_side.x + i, _side.y + j, _side.y + j + 1, _side.z + k, alpha);
					edges_OY[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ _side.x + i, edges_OY[i][j][k].value, _side.z + k });
				}
				else if (!matrix_3d[i][j][k] && matrix_3d[i][j + 1][k])
				{
					edges_OY[i][j][k].value = m_func_y(_side.x + i, _side.y + j, _side.y + j + 1, _side.z + k, alpha);
					edges_OY[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ _side.x + i, edges_OY[i][j][k].value, _side.z + k });
				}
				else
				{
					edges_OY[i][j][k].value = 0;
					edges_OY[i][j][k].n = 0;
				}
			}
		}
	}
	//OX
	for (int i = 0; i < size_block.x - 1; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k])
				{
					edges_OX[i][j][k].value = m_func_x(_side.x + i, _side.x + i + 1, _side.y + j, _side.z + k, alpha);
					edges_OX[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ edges_OX[i][j][k].value,  _side.y + j, _side.z + k });
				}
				else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k])
				{
					edges_OX[i][j][k].value = m_func_x(_side.x + i, _side.x + i + 1, _side.y + j, _side.z + k, alpha);
					edges_OX[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ edges_OX[i][j][k].value,  _side.y + j, _side.z + k });
				}
				else
				{
					edges_OX[i][j][k].value = 0;
					edges_OX[i][j][k].n = 0;
				}
			}
		}
	}
	//OZ
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z - 1; k++)
			{
				if (matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1])
				{
					edges_OZ[i][j][k].value = m_func_z(_side.x + i, _side.y + j, _side.z + k, _side.z + k + 1, alpha);
					edges_OZ[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ _side.x + i, _side.y + j, edges_OZ[i][j][k].value });
				}
				else if (!matrix_3d[i][j][k] && matrix_3d[i][j][k + 1])
				{
					edges_OZ[i][j][k].value = m_func_z(_side.x + i, _side.y + j, _side.z + k, _side.z + k + 1, alpha);
					edges_OZ[i][j][k].n = n;

					n++;
					fig->points.push_back(glm::vec3{ _side.x + i, _side.y + j, edges_OZ[i][j][k].value });
				}
				else
				{
					edges_OZ[i][j][k].value = 0;
					edges_OZ[i][j][k].n = 0;
				}
			}
		}
	}
}

void mesh_generation::check_polygons(figure* fig)
{
	for (int i = 0; i < size_block.x - 1; i++)
	{
		for (int j = 0; j < size_block.y - 1; j++)
		{
			for (int k = 0; k < size_block.z - 1; k++)
			{
				int n = 0;
				//angles
				if (true)
				{
					//angle 0, 0, 0
					if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 1, 0, 0
					if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j + 1][k])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OY[i + 1][j][k].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j + 1][k])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 1, 0, 1
					if (!matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j][k + 1].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j][k + 1].n;
						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 0, 0, 1
					if (!matrix_3d[i][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j][k + 1].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j][k + 1].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 0, 1, 0
					if (matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j][k])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					else if (!matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j][k])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 1, 1, 0
					if (!matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j][k])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OY[i + 1][j][k].n;
						p1.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j][k])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OY[i + 1][j][k].n;
						p1.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 1, 1, 1
					if (!matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					//angle 0, 1, 1
					if (!matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
					else if (matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
					}
				}
				//edges
				if (true)
				{
					//edge OY 0, 0
					if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OY 1, 0
					if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i + 1][j][k].n;
						p1.x = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.x = edges_OZ[i + 1][j][k].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i + 1][j][k].n;
						p1.y = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.y = edges_OZ[i + 1][j][k].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OY 1, 1
					if (!matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i + 1][j][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.y = edges_OZ[i + 1][j][k].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i + 1][j][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.x = edges_OZ[i + 1][j][k].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OY 0, 1
					if (!matrix_3d[i][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}

					//edge OX 0, 0
					if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OX 1, 0
					if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k]
						&& matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j + 1][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OZ[i][j + 1][k].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k]
						&& !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j + 1][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OZ[i][j + 1][k].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OX 1, 1
					if (!matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j + 1][k].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OZ[i][j + 1][k].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j + 1][k].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OZ[i][j + 1][k].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OX 0, 1
					if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}

					//edge OZ 0, 0
					if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.y = edges_OX[i][j][k].n;
						p2.x = edges_OX[i][j][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.x = edges_OX[i][j][k].n;
						p2.y = edges_OX[i][j][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OZ 1, 0
					if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OY[i + 1][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OX[i][j][k].n;
						p2.y = edges_OX[i][j][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OX[i][j][k].n;
						p2.x = edges_OX[i][j][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OZ 1, 1
					if (!matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OY[i + 1][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1]
						&& matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OY[i + 1][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					//edge OZ 0, 1
					if (!matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OY[i][j][k].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OY[i][j][k].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);
					}
				}
				//OY polygon 1
				if (true)
				{
					if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OY[i][j][k].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OY[i][j][k].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OX[i][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OX[i][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j + 1][k].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OZ[i][j + 1][k].n;
						p2.y = edges_OX[i][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j + 1][k].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OZ[i][j + 1][k].n;
						p2.x = edges_OX[i][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OY polygon 2
				if (true)
				{
					if (!matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						!matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						!matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k + 1].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OX[i][j][k + 1].n;
						p2.y = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k + 1].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OX[i][j][k + 1].n;
						p2.x = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j][k] && !matrix_3d[i][j][k + 1] &&
						!matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.y = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j][k] && matrix_3d[i][j][k + 1] &&
						matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.x = edges_OY[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}

				}
				//OY polygon 3
				if (n == 0)
				{
					if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OY[i][j][k].n;
						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OY[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OY[i][j][k].n;

						p2.y = edges_OZ[i + 1][j][k].n;
						p2.z = edges_OX[i][j][k].n;
						p2.x = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.y = edges_OZ[i + 1][j][k].n;
						p2.x = edges_OX[i][j][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.x = edges_OX[i][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OY[i][j][k].n;

						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OX[i][j + 1][k].n;
						p2.x = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OY polygon 4
				if (n != 2)
				{
					if (!matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OY[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OY[i][j][k].n;
						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.y = edges_OX[i][j][k + 1].n;
						p2.x = edges_OZ[i][j][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OY[i][j][k].n;

						p2.y = edges_OX[i][j][k + 1].n;
						p2.z = edges_OZ[i][j][k].n;
						p2.x = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] &&
						matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OY[i][j][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OZ[i][j + 1][k].n;
						p2.x = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] &&
						!matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j + 1][k].n;
						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OX polygon 1
				if (n == 0)
				{
					if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OX polygon 2
				if (n != 2)
				{
					if (matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OY[i][j][k + 1].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OZ[i][j + 1][k].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j + 1][k] && matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OZ[i + 1][j + 1][k].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OX polygon 3
				if (n == 0)
				{
					if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1]
						&& !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j + 1][k].n;
						p1.x = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1]
						&& matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j + 1][k].n;
						p1.y = edges_OX[i][j][k].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1]
						&& !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.y = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1]
						&& matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OY[i][j][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i][j + 1][k + 1]
						&& matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i][j + 1][k + 1]
						&& !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.y = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i + 1][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OX polygon 4
				if (n != 2)
				{
					if (matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OX[i][j][k + 1].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						p2.x = edges_OZ[i][j][k].n;
						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j][k] && matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OZ[i][j][k].n;

						p2.y = edges_OZ[i][j][k].n;
						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j + 1][k + 1] && !matrix_3d[i][j][k] && !matrix_3d[i][j][k + 1]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						p2.y = edges_OZ[i + 1][j][k].n;
						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j + 1][k + 1] && matrix_3d[i][j][k] && matrix_3d[i][j][k + 1]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OX[i][j + 1][k + 1].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						p2.x = edges_OZ[i + 1][j][k].n;
						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.z = edges_OY[i + 1][j][k + 1].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OZ polygon 1
				if (n == 0)
				{
					if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.y = edges_OX[i][j][k].n;
						p2.x = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.x = edges_OX[i][j][k].n;
						p2.y = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& !matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OX[i][j][k + 1].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.x = edges_OX[i][j][k + 1].n;
						p2.y = edges_OY[i][j][k + 1].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OX[i][j][k + 1].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.y = edges_OX[i][j][k + 1].n;
						p2.x = edges_OY[i][j][k + 1].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OZ polygon 2
				if (n != 2)
				{
					if (matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& !matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OY[i + 1][j][k].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OY[i + 1][j][k].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OY[i + 1][j][k].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i + 1][j + 1][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j][k]
						&& !matrix_3d[i + 1][j + 1][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OY[i + 1][j][k + 1].n;
						p1.y = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.y = edges_OY[i + 1][j][k + 1].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i + 1][j + 1][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j][k]
						&& matrix_3d[i + 1][j + 1][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OY[i + 1][j][k + 1].n;
						p1.x = edges_OZ[i + 1][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OY[i + 1][j][k + 1].n;
						p2.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OZ polygon 3
				if (n == 0)
				{
					if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.x = edges_OZ[i][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.y = edges_OZ[i][j][k].n;
						p1.z = edges_OZ[i][j + 1][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.x = edges_OX[i][j + 1][k].n;
						p2.y = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OX[i][j + 1][k].n;

						p2.y = edges_OX[i][j + 1][k].n;
						p2.x = edges_OY[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i][j + 1][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && !matrix_3d[i][j + 1][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.y = edges_OX[i][j + 1][k + 1].n;
						p2.x = edges_OY[i][j][k + 1].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i][j + 1][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && matrix_3d[i][j + 1][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OX[i][j + 1][k + 1].n;

						p2.x = edges_OX[i][j + 1][k + 1].n;
						p2.y = edges_OY[i][j][k + 1].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
				//OZ polygon 4
				if (n != 2)
				{
					if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.y = edges_OZ[i][j][k].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1;

						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.x = edges_OZ[i][j][k].n;
						p1.z = edges_OZ[i + 1][j][k].n;

						fig->polygons.push_back(p1);

						n++;
					}
					else if (matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.y = edges_OY[i + 1][j][k].n;
						p2.x = edges_OX[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k].n;

						p2.x = edges_OY[i + 1][j][k].n;
						p2.y = edges_OX[i][j][k].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (!matrix_3d[i][j][k] && !matrix_3d[i + 1][j][k] && !matrix_3d[i + 1][j + 1][k]
						&& matrix_3d[i][j][k + 1] && !matrix_3d[i + 1][j][k + 1] && matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.x = edges_OZ[i][j][k].n;
						p1.y = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.x = edges_OY[i + 1][j][k + 1].n;
						p2.y = edges_OX[i][j][k + 1].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
					else if (matrix_3d[i][j][k] && matrix_3d[i + 1][j][k] && matrix_3d[i + 1][j + 1][k]
						&& !matrix_3d[i][j][k + 1] && matrix_3d[i + 1][j][k + 1] && !matrix_3d[i + 1][j + 1][k + 1])
					{
						glm::ivec3 p1, p2;

						p1.y = edges_OZ[i][j][k].n;
						p1.x = edges_OZ[i + 1][j + 1][k].n;
						p1.z = edges_OY[i + 1][j][k + 1].n;

						p2.y = edges_OY[i + 1][j][k + 1].n;
						p2.x = edges_OX[i][j][k + 1].n;
						p2.z = edges_OZ[i][j][k].n;

						fig->polygons.push_back(p1);
						fig->polygons.push_back(p2);

						n++;
					}
				}
			}
		}
	}
}

void mesh_generation::func_thr(glm::ivec3 _size_block, glm::ivec3 _side, glm::ivec3 _size, int _n_thr, int _x)
{
	mesh_generation m_g(_size_block);
	m_g.create_points_matrix();
	m_g.create_edges_matrix();

	for (int i = 0; i < _size.x; i++)
	{
		for (int j = 0; j < _size.y; j++)
		{
			for (int k = 0; k < _size.z; k++)
			{
				if ((i + (j + k*_size.y)*_size.x) % _n_thr == _x)
				{
					figure fig;
					glm::ivec3 side;

					side.x = _side.x + i * (_size_block.x - 1);
					side.y = _side.y + j * (_size_block.y - 1);
					side.z = _side.z + k * (_size_block.z - 1);
					m_g.gen_mesh(&fig, side);

					lock_fig.lock();

					{
						int n_p = n_points;
						for (const auto& element : fig.polygons)
						{
							glm::ivec3 p = element;

							p.x += n_points;
							p.y += n_points;
							p.z += n_points;

							base_fig->polygons.push_back(p);
						}
					}

					{
						for (const auto& element : fig.points)
						{
							base_fig->points.push_back(element);
						}
						n_points += fig.points.size();
					}

					lock_fig.unlock();
				}
			}
		}
	}

	m_g.delete_points_matrix();
	m_g.delete_edges_matrix();

	compl_thr++;
}

float func(float _x, float _y, float _z)
{
	siv::PerlinNoise test(1);
	return test.octaveNoise0_1(_x / 32, _y / 32, _z / 32, 2) - 0.6;
}

float m_func_x(float _x1, float _x2, float _y, float _z, float alpha)
{
	float x, f1, f2;

	f1 = func(_x1, _y, _z);
	f2 = func(_x2, _y, _z);

	float _x = (_x1 + _x2) / 2;

	if (abs(abs(_x1) - abs(_x2)) < alpha)
	{
		return _x;
	}

	if (abs(f1) > abs(f2))
	{
		x = m_func_x(_x, _x2, _y, _z, alpha);
	}
	else
	{
		x = m_func_x(_x, _x1, _y, _z, alpha);
	}

	return x;
}

float m_func_y(float _x, float _y1, float _y2, float _z, float alpha)
{
	float y, f1, f2;

	f1 = func(_x, _y1, _z);
	f2 = func(_x, _y2, _z);

	float _y = (_y1 + _y2) / 2;

	if (abs(abs(_y1) - abs(_y2)) < alpha)
	{
		return _y;
	}

	if (abs(f1) < abs(f2))
	{
		y = m_func_y(_x, _y, _y1, _z, alpha);
	}
	else
	{
		y = m_func_y(_x, _y, _y2, _z, alpha);
	}

	return y;
}

float m_func_z(float _x, float _y, float _z1, float _z2, float alpha)
{
	float z, f1, f2;

	f1 = func(_x, _y, _z1);
	f2 = func(_x, _y, _z2);

	float _z = (_z1 + _z2) / 2;

	if (abs(abs(_z1) - abs(_z2)) < alpha)
	{
		return _z;
	}

	if (abs(f1) < abs(f2))
	{
		z = m_func_z(_x, _y, _z1, _z, alpha);
	}
	else
	{
		z = m_func_z(_x, _y, _z2, _z, alpha);
	}

	return z;
}
