#include <cstdio>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

#include "hull3d.hpp"

std::vector<Triangle> convex_hull(std::vector<R3>& points)
{

	std::sort(points.begin(), points.end());
	std::vector<Triangle> hull = hull3d(points);
	unsigned int cloud_size = hull.size();
	unsigned int hull_size = hull.size();

	// re-index both vertex buffer and triangle buffer
	unsigned int* pindex = (unsigned int*) malloc (cloud_size * sizeof(unsigned int));
	unsigned int* tindex = (unsigned int*) malloc (hull_size * sizeof(unsigned int));
	for (unsigned int i = 0; i < cloud_size; ++i) pindex[i] = 0;

	unsigned int t = 0;
	for(unsigned int i = 0; i < hull_size; ++i)
		if(hull[i].state > 0)
		{
			tindex[i] = t++;
			pindex[hull[i].vertices.x] = 1;
			pindex[hull[i].vertices.y] = 1;
			pindex[hull[i].vertices.z] = 1;
		}
		else
			tindex[i] = -1;

	unsigned int p = 0;
	FILE *f = fopen("vertices.txt", "w");
	fprintf(f, "Vertices used : \n\n");
	for(unsigned int i = 0; i < cloud_size; ++i)
		if (pindex[i])
		{
			pindex[i] = p++;
			fprintf(f, "%d(%d) : %.6f, %.6f, %.6f\n", i, pindex[i], points[i].position.x, points[i].position.y, points[i].position.z);
		}; 
	fclose(f);

	std::vector<Triangle> chull;
	for(unsigned int i = 0; i < hull_size; ++i)
		if(hull[i].state > 0)
		{
			Triangle triangle;
			triangle.id = tindex[i];
			triangle.state = 1;
			triangle.vertices = glm::ivec3(pindex[hull[i].vertices.x], pindex[hull[i].vertices.y], pindex[hull[i].vertices.z]);
			triangle.edges = glm::ivec3(tindex[hull[i].edges.x], tindex[hull[i].edges.y], tindex[hull[i].edges.z]);
			triangle.normal = hull[i].normal;
			chull.push_back(triangle);
		};

	free(pindex);
	free(tindex);
	return chull;
};


inline void attach_edge(Triangle& triangle, int p, int q, int id)
{
	if (triangle.vertices.x == p)
	{
		if (triangle.vertices.y == q) triangle.edges.z = id;
		else triangle.edges.y = id;
	}
	else if(triangle.vertices.y == p)
	{
		if (triangle.vertices.z == q) triangle.edges.x = id;
		else triangle.edges.z = id;
	}
	else // triangle.vertices.z == p
	{
		if (triangle.vertices.x == q) triangle.edges.y = id;
		else triangle.edges.x = id;
	};
};

std::vector<Triangle> hull3d(std::vector<R3>& points)
{
	std::vector<Triangle> hull;
	int hull_size = 0;

  	Triangle triangle;
  	triangle.state = USED;
  	triangle.vertices = glm::ivec3(0, 1, 2);

	glm::dvec3 mass_center = points[0].position + points[1].position + points[2].position;
	glm::dvec3 n = glm::cross(points[0].position - points[1].position, points[0].position - points[2].position);

	triangle.id = hull_size++;
	triangle.normal = n;
	triangle.edges = glm::ivec3(1);
	hull.push_back(triangle);

	triangle.id = hull_size++;
	triangle.normal = -n;
	triangle.edges = glm::ivec3(0);
	hull.push_back(triangle);

	for(int p = 3; p < (int) points.size(); ++p)
	{
		glm::dvec3 mass_center_direction = mass_center - double(p) * points[p].position;
		mass_center += points[p].position;

		int hsize = hull_size;

		std::vector<int> xlist;
    	int xlist_size = 0;

    	for(int h = hull_size - 1; h >= 0; --h)
    		if(glm::dot(points[p].position - points[hull[h].vertices.x].position, hull[h].normal) > 0.0)
    		{
    			hull[h].state = DISCARDED;
    			xlist.push_back(h);
    			xlist_size = 1;
    			break;
    		};

    	for(int index = 0; index < xlist_size; index++)
    	{
    		int id = xlist[index];
			int bc = hull[id].edges.x;
			int ca = hull[id].edges.y;
    		int ab = hull[id].edges.z;

			if (glm::dot(points[p].position - points[hull[bc].vertices.x].position, hull[bc].normal) > 0.0)
			{
				if(hull[bc].state == USED)
				{
					hull[bc].state = DISCARDED;
					xlist.push_back(bc);
					++xlist_size;
				};
			}
			else
			{
				triangle.id = hull_size++;
				triangle.state = NEEDS_ADJUSTMENT;
				triangle.vertices = glm::ivec3(p, hull[id].vertices.y, hull[id].vertices.z);
				triangle.edges = glm::ivec3(bc, -1, -1);
				triangle.normal = glm::cross(points[triangle.vertices.x].position - points[triangle.vertices.y].position, points[triangle.vertices.x].position - points[triangle.vertices.z].position);
				if (glm::dot(triangle.normal, mass_center_direction) > 0.0) triangle.normal = -triangle.normal;					
				attach_edge(hull[bc], hull[id].vertices.y, hull[id].vertices.z, triangle.id);
				hull.push_back(triangle);
			};

			if (glm::dot(points[p].position - points[hull[ca].vertices.x].position, hull[ca].normal) > 0.0)
			{
				if(hull[ca].state == USED)
				{
					hull[ca].state = DISCARDED;
					xlist.push_back(ca);
					++xlist_size;
				};
			}
			else
			{
				triangle.id = hull_size++;
				triangle.state = NEEDS_ADJUSTMENT;
				triangle.vertices = glm::ivec3(p, hull[id].vertices.x, hull[id].vertices.z);
				triangle.edges = glm::ivec3(ca, -1, -1);
				triangle.normal = glm::cross(points[triangle.vertices.x].position - points[triangle.vertices.y].position, points[triangle.vertices.x].position - points[triangle.vertices.z].position);
				if (glm::dot(triangle.normal, mass_center_direction) > 0.0) triangle.normal = -triangle.normal;
				attach_edge(hull[ca], hull[id].vertices.z, hull[id].vertices.x, triangle.id);
				hull.push_back(triangle);
			};

    		if(glm::dot(points[p].position - points[hull[ab].vertices.x].position, hull[ab].normal) > 0.0)
    		{
    			if(hull[ab].state == USED)
    			{
    				hull[ab].state = DISCARDED;
    				xlist.push_back(ab);
    				++xlist_size;
    			};
    		}
   			else
   			{
  				triangle.id = hull_size++;
  				triangle.state = NEEDS_ADJUSTMENT;
				triangle.vertices = glm::ivec3(p, hull[id].vertices.x, hull[id].vertices.y);
				triangle.edges = glm::ivec3(ab, -1, -1);
				triangle.normal = glm::cross(points[triangle.vertices.x].position - points[triangle.vertices.y].position, points[triangle.vertices.x].position - points[triangle.vertices.z].position);
				if (glm::dot(triangle.normal, mass_center_direction) > 0.0) triangle.normal = -triangle.normal;
				attach_edge(hull[ab], hull[id].vertices.x, hull[id].vertices.y, triangle.id);
				hull.push_back(triangle);
			};
		};

		std::vector<Snork> norks;
		Snork snork;
		for(int q = hull_size - 1; q >= hsize; q--)
		{
			if(hull[q].state == NEEDS_ADJUSTMENT)
			{
				snork.id = q;
				snork.a = hull[q].vertices.y;
				snork.b = 1;
				norks.push_back(snork);
				snork.a = hull[q].vertices.z;
				snork.b = 0;
				norks.push_back(snork);
				hull[q].state = USED;
			};
			std::sort(norks.begin(), norks.end());
			for(int s = 0; s < (int) norks.size() - 1; s++)
				if(norks[s].a == norks[s + 1].a)
				{
					if(norks[s].b == 1)
						hull[norks[s].id].edges.z = norks[s + 1].id;
					else
						hull[norks[s].id].edges.y = norks[s + 1].id;

					if(norks[s + 1].b == 1)
						hull[norks[s + 1].id].edges.z = norks[s].id;
					else
						hull[norks[s + 1].id].edges.y = norks[s].id;
				};
		};
	};
	return hull;
};