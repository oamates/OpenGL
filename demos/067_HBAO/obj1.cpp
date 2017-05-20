#include "log.hpp"
#include "obj1.hpp"

#include <iostream>
#include <string>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <fstream>
#include <vector>
#include <list>

typedef struct
{
	int position;
	int normal;
	int texcoord;
	int sg;
	unsigned int index;
}sVertexIndex;

typedef struct
{
	float x, y, z;
}sVec3;

typedef struct
{
	float x, y;
}sVec2;

bool equal(const sVertexIndex &lhs, const sVertexIndex &rhs);

unsigned int insertVertexIndex(	sVertexIndex vertex,
								std::vector<sVertexIndex> &table,
								std::vector<std::list<sVertexIndex> > &existingTable);

void readPosition(	const std::string &line,
					std::vector<sVec3> &table,
					float scale);

void readNormal(	const std::string &line,
					std::vector<sVec3> &table );

void readTexcoord(	const std::string &line,
					std::vector<sVec2> &table );

void readFace(		const std::string &line,
					std::vector<sVertexIndex> &vertexTable,
					std::vector<std::list<sVertexIndex> > &existingTable,
					std::vector<Mesh::sFace> &faceTable,
					int sg);

void readSG(		const std::string &line,
					int &sg);

void readG(			const std::string &line,
					std::string &name);

void fillMesh(		Mesh &mesh,
					const std::string &name,
					std::vector<sVertexIndex> &vertexTable,
					std::vector<Mesh::sFace> &faceTable,
					std::vector<sVec3> &positionTable,
					std::vector<sVec3> &normalTable,
					std::vector<sVec2> &texcoordTable);

Mesh loadMeshFromObj(const char *filename, float scale)
{
	debug_msg("Attempting to load mesh from %s ...", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	if(filehandle.fail())
	{
		debug_msg("Could not open file.");
		return Mesh();
	}

	std::vector<std::list<sVertexIndex> > existingVertexTable;

	std::vector<sVertexIndex>	vertexTable;
	std::vector<Mesh::sFace>	faceTable;

	std::vector<sVec3> positionTable;
	std::vector<sVec3> normalTable;
	std::vector<sVec2> texcoordTable;

	std::string line;
	std::string name(filename);
	int sg = 0;

	debug_msg("Reading data... ");

	while(filehandle.good() && !filehandle.eof())
	{
		std::getline(filehandle, line);
		if(line[0] == 'v')
		{
			if(line[1] == 't')
				readTexcoord(line, texcoordTable);
			else if(line[1] == 'n')
				readNormal(line, normalTable);
			else
				readPosition(line, positionTable, scale);
		}
		else if(line[0] == 'f')
			readFace(line, vertexTable, existingVertexTable, faceTable, sg);
		else if(line[0] == 's')
			readSG(line, sg);
	}

	Mesh m;
	fillMesh(	m, name, vertexTable, faceTable,
				positionTable, normalTable, texcoordTable);

	debug_msg("Done! Total vertex count = %d. Total face count = %d.", (int) vertexTable.size(), (int) faceTable.size());
	return m;
}

std::vector<Mesh> loadMeshesFromObj(const char *filename, float scale)
{
	debug_msg("Attempting to load meshes from %s ...", filename);

	std::ifstream filehandle;
	filehandle.open(filename, std::ios::in);

	std::vector<Mesh> meshes;

	if(filehandle.fail())
	{
		debug_msg("Could not open file.");
		return meshes;
	}

	std::vector<std::list<sVertexIndex> > existingVertexTable;

	std::vector<sVertexIndex>	vertexTable;
	std::vector<Mesh::sFace>	faceTable;

	std::vector<sVec3> positionTable;
	std::vector<sVec3> normalTable;
	std::vector<sVec2> texcoordTable;

	std::string line;
	std::string name;
	std::string material;
	int sg = 0;
	int count = 0;

	debug_msg("Reading data... ");

	while( filehandle.good() && !filehandle.eof() )
	{
		std::getline(filehandle, line);
		if(line[0] == 'v')
		{
			if(line[1] == 't')
				readTexcoord(line, texcoordTable);
			else if(line[1] == 'n')
				readNormal(line, normalTable);
			else
				readPosition(line, positionTable, scale);
		}
		else if(line[0] == 'f')
			readFace(line, vertexTable, existingVertexTable, faceTable, sg);
		else if(line[0] == 's')
			readSG(line, sg);
		else if(line[0] == 'g')
		{
			readG(line, name);
		}
		else if(line[0] == 'u')
		{
			char str[32];
			char mtl[128];
			int success = sscanf(line.c_str(), "%s %s", str, mtl);
			if(success && strcmp(str, "usemtl") == 0)
			{
				if(count > 0)
				{
					meshes.push_back(Mesh());
					fillMesh(	meshes[count-1], name, vertexTable, faceTable,
								positionTable, normalTable, texcoordTable);
					meshes[count-1].material = material;
					vertexTable.clear();
					faceTable.clear();
					existingVertexTable.clear();
				}
				++count;

				if(success > 1)
					material = std::string(mtl);
			}
		}
	}

	if(count > 0)
	{
		meshes.push_back(Mesh());
		fillMesh(meshes[count-1], name, vertexTable, faceTable, positionTable, normalTable, texcoordTable);
		meshes[count-1].material = material;
	}

	debug_msg("Done! %d meshes loaded.", (int) meshes.size());

	return meshes;
}

bool equal(const sVertexIndex &lhs, const sVertexIndex &rhs)
{
	return 	lhs.position == rhs.position &&
			lhs.normal == rhs.normal &&
			lhs.texcoord == rhs.texcoord &&
			lhs.sg == rhs.sg;
}

unsigned int insertVertexIndex(	sVertexIndex vertex,
								std::vector<sVertexIndex> &table,
								std::vector<std::list<sVertexIndex> > &existingTable)
{
	if(vertex.position == -1)
	{
		debug_msg("Bad vertex given: v %i %i %i \n", vertex.position, vertex.normal, vertex.texcoord);
		return 0;
	}

	vertex.index = table.size();

	// Check against existing vertices, uses a hashtable approach with vertexIndex as the
	// hashfunction, and variations of this vertexIndex is stored as a linked list in this slot.
	// If the 'hashindex' is greater than the size of the table, then expand the table.
	if(vertex.position >= (int)existingTable.size())
	{
		existingTable.resize(vertex.position+1, std::list<sVertexIndex>());
		existingTable[vertex.position].push_back(vertex);
	}
	else
	{
		// Check if the vertex already exists in the list
		std::list<sVertexIndex>::const_iterator it = existingTable[vertex.position].begin();
		while(it != existingTable[vertex.position].end())
		{
			if(equal(vertex, *it))
				return it->index;
			++it;
		}
		existingTable[vertex.position].push_back(vertex);
	}

	// No vertex was found, insert and return the index.
	table.push_back(vertex);
	return vertex.index;
}

void readPosition(const std::string &line, std::vector<sVec3> &table, float scale)
{
	sVec3 pos;
	sscanf(line.c_str(), "%*s %f %f %f", &pos.x, &pos.y, &pos.z);
	pos.x *= scale;
	pos.y *= scale;
	pos.z *= scale;
	table.push_back(pos);
}

void readNormal(const std::string &line, std::vector<sVec3> &table)
{
	sVec3 norm;
	sscanf(line.c_str(), "%*s %f %f %f", &norm.x, &norm.y, &norm.z);
	table.push_back(norm);
}

void readTexcoord(const std::string &line, std::vector<sVec2> &table)
{
	sVec2 texcoord;
	sscanf(line.c_str(), "%*s %f %f", &texcoord.x, &texcoord.y);
	table.push_back(texcoord);
}

void readFace(	const std::string &line,
				std::vector<sVertexIndex> &vertexTable,
				std::vector<std::list<sVertexIndex> > &existingTable,
				std::vector<Mesh::sFace> &faceTable,
				int sg )
{
	int position[4] = 	{-1,-1,-1,-1};
	int normal[4] 	= 	{-1,-1,-1,-1};
	int texcoord[4] = 	{-1,-1,-1,-1};
	int params[3] 	= 	{-1,-1,-1};
	int count = 0;

	// f 1 2 3 (4)
	// f 1/2 1/2 1/2 (1/2)
	// f 1/2/3 1/2/3 1/2/3 (1/2/3)

	// shared buffer for chunks, wierd bug if memory was allocated
	// as char c[4][100]
	char *buffer = new char[400];
	char *c[4] = {&buffer[0], &buffer[100], &buffer[200], &buffer[300]};

	count = sscanf(line.c_str(), "%*s %s %s %s %s", c[0], c[1], c[2], c[3]);

	for(int i = 0; i < count; ++i)
	{
		int p = sscanf(c[i], "%i/%i/%i", &params[0], &params[1], &params[2]);
		position[i] = (p > 0 && params[0] > 0) ? params[0]-1 : -1;
		texcoord[i] = (p > 1 && params[1] > 0) ? params[1]-1 : -1;
		normal[i] 	= (p > 2 && params[2] > 0) ? params[2]-1 : -1;
	}

	delete[] buffer;

	if(count == 3 || count == 4)
	{
		sVertexIndex vertex;
		Mesh::sFace face;
		unsigned int indices[4] = {0,0,0,0};

		for(int i = 0; i < count; ++i)
		{
			vertex.position = position[i];
			vertex.normal 	= normal[i];
			vertex.texcoord = texcoord[i];
			indices[i] = insertVertexIndex(vertex, vertexTable, existingTable);
		}

		face.v[0] = indices[0];
		face.v[1] = indices[1];
		face.v[2] = indices[2];

		faceTable.push_back(face);

		if(count == 4)
		{
			face.v[0] = indices[3];
			face.v[1] = indices[0];
			face.v[2] = indices[2];
			faceTable.push_back(face);
		}
	}
}

void readSG(const std::string& line, int& sg)
{
	sscanf(line.c_str(), "%*s %i", &sg);
}

void readG(const std::string &line, std::string &name)
{
	char *str = new char[128];
	int count = sscanf(line.c_str(), "%*s %s", str);
	if(count > 0) name = std::string(str);
	delete[] str;
}

void fillMesh(	Mesh &mesh,
				const std::string &name,
				std::vector<sVertexIndex> &vertexTable,
				std::vector<Mesh::sFace> &faceTable,
				std::vector<sVec3> &positionTable,
				std::vector<sVec3> &normalTable,
				std::vector<sVec2> &texcoordTable )
{
	mesh.name = name;

	for(unsigned int i=0; i<vertexTable.size(); ++i)
	{
		int pos, norm, tc;
		pos = vertexTable[i].position;
		norm = vertexTable[i].normal;
		tc = vertexTable[i].texcoord;

		mesh.vertices.push_back(Mesh::sVertex());

		if(pos > -1 && pos < (int)positionTable.size())
		{
			mesh.vertices[i].x = positionTable[pos].x;
			mesh.vertices[i].y = positionTable[pos].y;
			mesh.vertices[i].z = positionTable[pos].z;
		}
		else
		{
			mesh.vertices[i].x = 0.0f;
			mesh.vertices[i].y = 0.0f;
			mesh.vertices[i].z = 0.0f;
		}

		if(norm > -1 && norm < (int)normalTable.size())
		{
			mesh.vertices[i].nx = normalTable[norm].x;
			mesh.vertices[i].ny = normalTable[norm].y;
			mesh.vertices[i].nz = normalTable[norm].z;
		}
		else
		{
			mesh.vertices[i].nx = 0.0f;
			mesh.vertices[i].ny = 0.0f;
			mesh.vertices[i].nz = 0.0f;
		}

		if(tc > -1 && tc < (int)texcoordTable.size())
		{
			mesh.vertices[i].u = texcoordTable[tc].x;
			mesh.vertices[i].v = texcoordTable[tc].y;
		}
		else
		{
			mesh.vertices[i].u = 0.0f;
			mesh.vertices[i].v = 0.0f;
		}
	}

	mesh.faces = faceTable;
}