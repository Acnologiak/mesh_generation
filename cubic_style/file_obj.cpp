#include "file_obj.h"

#include <sstream>

file_obj::file_obj(figure *_fig, std::string _name)
{
	name = _name;
	fig = _fig;
}


int file_obj::write_file()
{
	std::fstream file;
	file.open(name.c_str(), std::ios_base::out);
	std::string buffer, str;

	//write points 
	for (glm::vec3 p : fig->points)
	{
		file << "v " << p.x << " " << p.y << " " << p.z << std::endl;
	}

	//write v_normal
	/*for (glm::vec3 p : fig->v_normal)
	{
		file << "vn " << p.x << " " << p.y << " " << p.z << std::endl;
	}*/

	//write polygons
	int n = 1;
	for (glm::ivec3 p : fig->polygons)
	{
		//file << "f " << p.x << "//" << n << " " << p.y << "//" << n << " " << p.z << "//" << n << std::endl;
		file << "f " << p.x << " " << p.y << " " << p.z << std::endl;
		n++;
	}


	file.close();

	return 1;
}


int file_obj::read_file()
{
	std::ifstream file;
	std::string s;
	std::string::size_type s_size;
	int status = 0;
	int op = 0;
	glm::vec3 v_f;
	glm::ivec3 v_i;

	file.open(name.c_str(), std::ios_base::in);

	while (!file.eof())
	{
		file >> s;
		if (status == 0)
		{
			if (s == "v")
			{
				status = 1;
			}
			else if (s == "vn")
			{
				status = 2;
			}
			else if (s == "f")
			{
				status = 3;
			}
		}
		else if (status == 1)
		{
			if (op == 0)
			{
				v_f.x = atof(s.c_str());
				op = 1;
			}
			else if (op == 1)
			{
				v_f.y = atof(s.c_str());
				op = 2;
			}
			else if (op == 2)
			{
				v_f.z = atof(s.c_str());
				op = 0;
				status = 0;

				fig->points.push_back(v_f);
			}
		}
		else if (status == 2)
		{
			if (op == 0)
			{
				v_f.x = atof(s.c_str());
				op = 1;
			}
			else if (op == 1)
			{
				v_f.y = atof(s.c_str());
				op = 2;
			}
			else if (op == 2)
			{
				v_f.z = atof(s.c_str());
				op = 0;
				status = 0;

				fig->v_normal.push_back(v_f);
			}
		}
		else if (status == 3)
		{
			if (op == 0)
			{
				char str[64];
				char number[16];

				strcpy(str, s.c_str());
				for (int i = 0; i < strlen(str); i++)
				{
					if (str[i] != '/')
					{
						number[i] = str[i];
					}
					else
					{
						number[i] = '\0';
						break;
					}
				}

				v_i.x = atof(number);

				op = 1;
			}
			else if (op == 1)
			{
				char str[64];
				char number[16];

				strcpy(str, s.c_str());

				for (int i = 0; i < strlen(str); i++)
				{
					if (str[i] != '/')
					{
						number[i] = str[i];
					}
					else
					{
						number[i] = '\0';
						break;
					}
				}

				v_i.y = atof(number);
				op = 2;
			}
			else if (op == 2)
			{
				char str[64];
				char number[16];

				strcpy(str, s.c_str());

				for (int i = 0; i < strlen(str); i++)
				{
					if (str[i] != '/')
					{
						number[i] = str[i];
					}
					else
					{
						number[i] = '\0';
						break;
					}
				}

				v_i.z = atof(number);
				op = 0;
				status = 0;

				fig->polygons.push_back(v_i);
			}
		}

	}

	file.close();

	return 1;
}