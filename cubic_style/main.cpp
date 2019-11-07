#include <iostream>

#include "mesh_generation.h"
#include "file_obj.h"

int main()
{
	figure fig;
	glm::ivec3 size_block(33, 33, 33), side(-64, -64, -64), size(4, 4, 4);
	
	mesh_generation m_g(size_block);

	{
		std::chrono::time_point<std::chrono::system_clock> begin;
		std::chrono::time_point<std::chrono::system_clock> end;
		int second, milliseconds;

		begin = std::chrono::system_clock::now();

		m_g.gen_mesh_thr(&fig, side, size, std::thread::hardware_concurrency());

		end = std::chrono::system_clock::now();
		second = std::chrono::duration_cast<std::chrono::seconds> (end - begin).count();
		milliseconds = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() - second * 1000;
		std::cout << second << " " << milliseconds << std::endl;
	}

	{
		file_obj test(&fig, "test.obj");
		test.write_file();
	}

	system("pause");
	return 0;
}