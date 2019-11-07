#pragma once
#pragma warning(disable : 4996)

#include <iostream>
#include <string>
#include <fstream>

#include "mesh_generation.h"


class file_obj
{
public:
	file_obj(figure *_fig, std::string _name);

	int write_file();
	int read_file();
private:
	std::string name;
	figure *fig;
};