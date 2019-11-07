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
	func(fig, _side);
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

void mesh_generation::create_matrix()
{
	point_matrix** matrix_2d;
	matrix_3d = new point_matrix * *[size_block.x];
	for (int i = 0; i < size_block.x; i++)
	{
		matrix_2d = new point_matrix * [size_block.y];
		for (int j = 0; j < size_block.y; j++)
		{
			matrix_2d[j] = new point_matrix[size_block.z];
		}
		matrix_3d[i] = matrix_2d;
	}
}

void mesh_generation::delete_matrix()
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

void mesh_generation::func(figure* fig, glm::ivec3 _side)
{
	float f;
	siv::PerlinNoise test(1);
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				f = 32*test.octaveNoise0_1((float)(i + _side.x) / 96, (float)(j + _side.y) / 96, (float)(k + _side.z) / 96, 16) - 16;
				//f = pow(i + _side.x, 2) + pow(j + _side.y, 2) + pow(k + _side.z, 2) - 4000;
				if (f < 0)
				{
					matrix_3d[i][j][k].point_position = true;
				}
				else
				{
					matrix_3d[i][j][k].point_position = false;
				}
				matrix_3d[i][j][k].number = 0;
				matrix_3d[i][j][k].point_foreign = false;
			}
		}
	}
}

void mesh_generation::check_points(figure* fig, glm::ivec3 _side)
{
	//видалення проблемних точок
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z - 2; k++)
			{
				if (!matrix_3d[i][j][k].point_position && !matrix_3d[i][j][k + 2].point_position && matrix_3d[i][j][k + 1].point_position)
				{
					matrix_3d[i][j][k + 1].point_position = false;
				}
			}
		}
	}
	for (int k = 0; k < size_block.z; k++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int i = 0; i < size_block.x - 2; i++)
			{
				if (!matrix_3d[i][j][k].point_position && !matrix_3d[i + 2][j][k].point_position && matrix_3d[i + 1][j][k].point_position)
				{
					matrix_3d[i + 1][j][k].point_position = false;
				}
			}
		}
	}
	for (int i = 0; i < size_block.x; i++)
	{
		for (int k = 0; k < size_block.z; k++)
		{
			for (int j = 0; j < size_block.y - 2; j++)
			{
				if (!matrix_3d[i][j][k].point_position && !matrix_3d[i][j + 2][k].point_position && matrix_3d[i][j + 1][k].point_position)
				{
					matrix_3d[i][j + 1][k].point_position = false;
				}
			}
		}
	}
	//пошук граничних точок
	bool*** box;
	{
		box = new bool** [2];
		for (int i = 0; i < 2; i++)
		{
			box[i] = new bool* [2];
			for (int j = 0; j < 2; j++)
			{
				box[i][j] = new bool[2];
			}
		}
	}
	for (int i = 0; i < size_block.x - 1; i++)
	{
		for (int j = 0; j < size_block.y - 1; j++)
		{
			for (int k = 0; k < size_block.z - 1; k++)
			{
				for (int x = 0; x < 2; x++)
				{
					for (int y = 0; y < 2; y++)
					{
						for (int z = 0; z < 2; z++)
						{
							box[x][y][z] = true;
						}
					}
				}

				if (!matrix_3d[i][j][k].point_position)
				{
					box[0][0][0] = false;
				}
				if (!matrix_3d[i + 1][j][k].point_position)
				{
					box[1][0][0] = false;
				}
				if (!matrix_3d[i][j + 1][k].point_position)
				{
					box[0][1][0] = false;
				}
				if (!matrix_3d[i + 1][j + 1][k].point_position)
				{
					box[1][1][0] = false;
				}
				if (!matrix_3d[i][j][k + 1].point_position)
				{
					box[0][0][1] = false;
				}
				if (!matrix_3d[i + 1][j][k + 1].point_position)
				{
					box[1][0][1] = false;
				}
				if (!matrix_3d[i][j + 1][k + 1].point_position)
				{
					box[0][1][1] = false;
				}
				if (!matrix_3d[i + 1][j + 1][k + 1].point_position)
				{
					box[1][1][1] = false;
				}

				bool s = false;
				for (int x = 0; x < 2; x++)
				{
					for (int y = 0; y < 2; y++)
					{
						for (int z = 0; z < 2; z++)
						{
							if (box[x][y][z] == false)
							{
								s = true;
							}
						}
					}
				}

				if (s == true)
				{
					for (int x = 0; x < 2; x++)
					{
						for (int y = 0; y < 2; y++)
						{
							for (int z = 0; z < 2; z++)
							{
								if (box[x][y][z] == true)
								{
									matrix_3d[i + x][j + y][k + z].point_foreign = true;
								}
							}
						}
					}
				}
			}
		}
	}
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				delete[] box[j][k];
			}
		}
		for (int k = 0; k < 2; k++)
		{
			delete[] box[k];
		}
		delete[] box;
	}
	//n
	int n = 1;
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				if (matrix_3d[i][j][k].point_foreign)
				{
					matrix_3d[i][j][k].number = n;

					glm::ivec3 p;
					p.x = i + _side.x;
					p.y = j + _side.y;
					p.z = k + _side.z;
					fig->points.push_back(p);

					n++;
				}
			}
		}
	}
}

void mesh_generation::check_polygons(figure* fig)
{
	for (int i = 0; i < size_block.x; i++)
	{
		for (int j = 0; j < size_block.y - 1; j++)
		{
			for (int k = 0; k < size_block.z - 1; k++)
			{
				if (matrix_3d[i][j][k].point_foreign && matrix_3d[i][j + 1][k].point_foreign && matrix_3d[i][j][k + 1].point_foreign && matrix_3d[i][j + 1][k + 1].point_foreign)
				{
					glm::ivec3 polygon_1, polygon_2;

					if (i != 0)
					{
						if (matrix_3d[i - 1][j][k].point_position && matrix_3d[i - 1][j + 1][k].point_position && matrix_3d[i - 1][j][k + 1].point_position && matrix_3d[i - 1][j + 1][k + 1].point_position)
						{
							polygon_1.y = matrix_3d[i][j + 1][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.y = matrix_3d[i][j + 1][k + 1].number;
							polygon_2.x = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
						else
						{
							polygon_1.x = matrix_3d[i][j + 1][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.x = matrix_3d[i][j + 1][k + 1].number;
							polygon_2.y = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
					}
					else
					{
						if (matrix_3d[i + 1][j][k].point_position && matrix_3d[i + 1][j + 1][k].point_position && matrix_3d[i + 1][j][k + 1].point_position && matrix_3d[i + 1][j + 1][k + 1].point_position)
						{
							polygon_1.x = matrix_3d[i][j + 1][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.x = matrix_3d[i][j + 1][k + 1].number;
							polygon_2.y = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
						else
						{
							polygon_1.y = matrix_3d[i][j + 1][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.y = matrix_3d[i][j + 1][k + 1].number;
							polygon_2.x = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
					}

					fig->polygons.emplace_back(polygon_1);
					fig->polygons.emplace_back(polygon_2);
				}
			}
		}
	}
	for (int i = 0; i < size_block.x - 1; i++)
	{
		for (int j = 0; j < size_block.y; j++)
		{
			for (int k = 0; k < size_block.z - 1; k++)
			{
				if (matrix_3d[i][j][k].point_foreign && matrix_3d[i + 1][j][k].point_foreign && matrix_3d[i][j][k + 1].point_foreign && matrix_3d[i + 1][j][k + 1].point_foreign)
				{
					glm::ivec3 polygon_1, polygon_2;

					if (j != 0)
					{
						if (matrix_3d[i][j - 1][k].point_position && matrix_3d[i + 1][j - 1][k].point_position && matrix_3d[i][j - 1][k + 1].point_position && matrix_3d[i + 1][j - 1][k + 1].point_position)
						{
							polygon_1.x = matrix_3d[i + 1][j][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.x = matrix_3d[i + 1][j][k + 1].number;
							polygon_2.y = matrix_3d[i + 1][j][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
						else
						{
							polygon_1.y = matrix_3d[i + 1][j][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.y = matrix_3d[i + 1][j][k + 1].number;
							polygon_2.x = matrix_3d[i + 1][j][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
					}
					else
					{
						if (matrix_3d[i][j + 1][k].point_position && matrix_3d[i + 1][j + 1][k].point_position && matrix_3d[i][j + 1][k + 1].point_position && matrix_3d[i + 1][j + 1][k + 1].point_position)
						{
							polygon_1.y = matrix_3d[i + 1][j][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.y = matrix_3d[i + 1][j][k + 1].number;
							polygon_2.x = matrix_3d[i + 1][j][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
						else
						{
							polygon_1.x = matrix_3d[i + 1][j][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i][j][k + 1].number;

							polygon_2.x = matrix_3d[i + 1][j][k + 1].number;
							polygon_2.y = matrix_3d[i + 1][j][k].number;
							polygon_2.z = matrix_3d[i][j][k + 1].number;
						}
					}

					fig->polygons.emplace_back(polygon_1);
					fig->polygons.emplace_back(polygon_2);
				}
			}
		}
	}
	for (int i = 0; i < size_block.x - 1; i++)
	{
		for (int j = 0; j < size_block.y - 1; j++)
		{
			for (int k = 0; k < size_block.z; k++)
			{
				if (matrix_3d[i][j][k].point_foreign && matrix_3d[i][j + 1][k].point_foreign && matrix_3d[i + 1][j][k].point_foreign && matrix_3d[i + 1][j + 1][k].point_foreign)
				{
					glm::ivec3 polygon_1, polygon_2;

					if (k != 0)
					{
						if (matrix_3d[i][j][k - 1].point_position && matrix_3d[i + 1][j][k - 1].point_position && matrix_3d[i][j + 1][k - 1].point_position && matrix_3d[i + 1][j + 1][k - 1].point_position)
						{
							polygon_1.x = matrix_3d[i][j + 1][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i + 1][j][k].number;

							polygon_2.x = matrix_3d[i + 1][j + 1][k].number;
							polygon_2.y = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i + 1][j][k].number;
						}
						else
						{
							polygon_1.y = matrix_3d[i][j + 1][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i + 1][j][k].number;

							polygon_2.y = matrix_3d[i + 1][j + 1][k].number;
							polygon_2.x = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i + 1][j][k].number;
						}
					}
					else
					{
						if (matrix_3d[i][j][k + 1].point_position && matrix_3d[i + 1][j][k + 1].point_position && matrix_3d[i][j + 1][k + 1].point_position && matrix_3d[i + 1][j + 1][k + 1].point_position)
						{
							polygon_1.y = matrix_3d[i][j + 1][k].number;
							polygon_1.x = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i + 1][j][k].number;

							polygon_2.y = matrix_3d[i + 1][j + 1][k].number;
							polygon_2.x = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i + 1][j][k].number;
						}
						else
						{
							polygon_1.x = matrix_3d[i][j + 1][k].number;
							polygon_1.y = matrix_3d[i][j][k].number;
							polygon_1.z = matrix_3d[i + 1][j][k].number;

							polygon_2.x = matrix_3d[i + 1][j + 1][k].number;
							polygon_2.y = matrix_3d[i][j + 1][k].number;
							polygon_2.z = matrix_3d[i + 1][j][k].number;
						}
					}

					fig->polygons.emplace_back(polygon_1);
					fig->polygons.emplace_back(polygon_2);
				}
			}
		}
	}
}

void mesh_generation::func_thr(glm::ivec3 _size_block, glm::ivec3 _side, glm::ivec3 _size, int _n_thr, int _x)
{
	mesh_generation m_g(_size_block);
	m_g.create_matrix();

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

	m_g.delete_matrix();

	compl_thr++;
}

