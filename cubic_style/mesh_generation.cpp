#include "mesh_generation.h"

#include "perlin_noise.cpp"

mesh_generation::mesh_generation()
{
}


mesh_generation::~mesh_generation()
{
}

bool mesh_generation::m_gen(figure &fig, glm::ivec3 p1, glm::ivec3 p2, int _number_thr)
{
	if (status == 0)
	{
		status = 1;
		
		//Встановлення меж генерації меша
		{
			size.x = abs(p1.x - p2.x) + 1;
			size.y = abs(p1.y - p2.y) + 1;
			size.z = abs(p1.z - p2.z) + 1;

			side = p1;
		}

		//створення нової 3d матриці
		create_matrix();

		//запуск потоків для генерації
		{
			for (int i = 0; i < _number_thr; i++)
			{
				std::thread thr(&mesh_generation::thr_func, this, _number_thr, i);
				thr.detach();
			}
		}

		{
			int second, milliseconds;
			std::chrono::time_point<std::chrono::system_clock> begin;
			std::chrono::time_point<std::chrono::system_clock> end;
			begin = std::chrono::system_clock::now();

			for (;;)
			{
				if (status == 2)
				{
					end = std::chrono::system_clock::now();
					second = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
					milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() - second * 1000;
					std::cout << second << " " << milliseconds << std::endl;

					//очищення пам'яті від старої 3d матриці
					delete_matrix();

					break;
				}
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}

			
		}

		status = 0;

		return true;
	}
	else
	{
		return false;
	}
}

void mesh_generation::pause_thr_func(int _number_thr)
{
	for (;;)
	{
		if (_number_thr == number_thr_complete)
		{
			number_thr_complete = 0;
			status++;
			break;
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void mesh_generation::thr_func(int _number_thr, int x)
{
	func(_number_thr, x);
	number_thr_complete++;
	//std::cout << "Thr " << x << " a" << std::endl;
	pause_thr_func(_number_thr);
}

void mesh_generation::func(int _number_thr, int x)
{
	int f;
	siv::PerlinNoise test(1);
	for (int i = 0; i < size.x; i++)
	{
		if (i%_number_thr == x)
		{
			for (int j = 0; j < size.y; j++)
			{
				for (int k = 0; k < size.z; k++)
				{
					//f = pow(side.x + i, 2) + pow(side.y + j, 2) + pow(side.z + k, 2);
					f = 32 * test.octaveNoise0_1((float)(side.x + i) / 16.0, (float)(side.y + j) / 16.0, (float)(side.z + k) / 16.0, 4) - 32.0;
					if (f < 0)
					{
						matrix_3d[i][j][k].point_position = true;
					}
				}
			}
		}
	}
}

void mesh_generation::create_matrix()
{
	point_matrix **matrix_2d;
	matrix_3d = new point_matrix**[size.x];
	for (int i = 0; i < size.x; i++)
	{
		matrix_2d = new point_matrix*[size.y];
		for (int j = 0; j < size.y; j++)
		{
			matrix_2d[j] = new point_matrix[size.z];
		}
		matrix_3d[i] = matrix_2d;
	}
}

void mesh_generation::delete_matrix()
{
	for (int i = 0; i < size.x; i++)
	{
		for (int j = 0; j < size.y; j++)
		{
			delete[] matrix_3d[i][j];
		}
	}
	for (int i = 0; i < size.x; i++)
	{
		delete[] matrix_3d[i];
	}
	delete[] matrix_3d;
}

