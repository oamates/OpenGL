//=======================================================================================================================================================================================================================
//  Solid structure methods implementation
//=======================================================================================================================================================================================================================
#include <cstdlib>
#include <algorithm>

#include <glm/gtx/string_cast.hpp>
#include <glm/ext.hpp> 

#include "solid.hpp"
#include "log.hpp"
#include "hull3d.hpp"

//=======================================================================================================================================================================================================================
// main function : builds convex hull of a set of points
//=======================================================================================================================================================================================================================
void solid::convex_hull(std::vector<glm::dvec3>& points)
{
	struct 
	{
        bool operator () (const glm::dvec3& lhs, const glm::dvec3& rhs)
        {   
			if (lhs.z < rhs.z) return true;
		    if (lhs.z > rhs.z) return false;
			if (lhs.y < rhs.y) return true;
		    if (lhs.y > rhs.y) return false;
			return lhs.x < rhs.x;
        }   
    } dvec3_lex;

	std::sort(points.begin(), points.end(), dvec3_lex);
	int cloud_size = (int) points.size();

	// ==================================================================================================================================================================================================================
	// the main call
	// ==================================================================================================================================================================================================================
	std::vector<Triangle> hull = hull3d(points);
	unsigned int hull_size = hull.size();

	// ==================================================================================================================================================================================================================
	// now just reindex both vertex and index buffers
	// ==================================================================================================================================================================================================================
	unsigned int* pindex = (unsigned int*) calloc (cloud_size, sizeof(unsigned int));
	unsigned int* tindex = (unsigned int*) malloc (hull_size * sizeof(unsigned int));
	
	V = 0; F = 0;

	for(unsigned int i = 0; i < hull_size; ++i)
		if(hull[i].state == USED)
		{
			tindex[i] = F++;
			pindex[hull[i].vertices.x] = 1;
			pindex[hull[i].vertices.y] = 1;
			pindex[hull[i].vertices.z] = 1;
		}
		else
			tindex[i] = -1;

	for(int i = 0; i < cloud_size; ++i)
		if (pindex[i]) pindex[i] = V++;

	// ==================================================================================================================================================================================================================
	// Euler's Formula
	// ==================================================================================================================================================================================================================
	E = V + F - 2;

	// ==================================================================================================================================================================================================================
	// copy vertices which are actually used in the convex hull  
	// ==================================================================================================================================================================================================================
	debug_msg("V = %d. E = %d. F = %d.", V, E, F);
	
	vertices.resize(V);
	vertices[0] = points[0];
	
	debug_msg("vertex[0] = %s", glm::to_string(vertices[0]).c_str());

	for(int i = 1; i < cloud_size; ++i)
		if (pindex[i]) 
		{
			vertices[pindex[i]] = points[i];
			debug_msg("vertex[%d] = %s", pindex[i], glm::to_string(points[i]).c_str());
		};

	// ==================================================================================================================================================================================================================
	// initialize volume, center of mass and second-order momenta with zero values
	// ==================================================================================================================================================================================================================
	volume = 0.0;
	mass_center = glm::dvec3(0.0);
	mxx = mxy = mxz = myy = myz = mzz = 0.0;

	// ==================================================================================================================================================================================================================
	// set up normals, indices and adjacent triangles indices
	// ==================================================================================================================================================================================================================
	normals.resize(V);
    triangles.resize(F); adjacency.resize(F);	
    int f = 0;
	for(unsigned int i = 0; i < hull_size; ++i) if(hull[i].state == USED)
	{
		int xi = pindex[hull[i].vertices.x];
		int yi = pindex[hull[i].vertices.y];
		int zi = pindex[hull[i].vertices.z];
		int xa = tindex[hull[i].edges.x];
		int ya = tindex[hull[i].edges.y];
		int za = tindex[hull[i].edges.z];

   		triangles[f] = glm::ivec3(xi, yi, zi);
   		adjacency[f] = glm::ivec3(xa, ya, za);

		debug_msg("triangle[%d] Vertices : (%d, %d, %d). Edges : (%d, %d, %d).", f, xi, yi, zi, xa, ya, za);


   		glm::dvec3 X = vertices[xi];
   		glm::dvec3 Y = vertices[yi];
   		glm::dvec3 Z = vertices[zi];

		// ==============================================================================================================================================================================================================
		// compute input of a face normal to the 3 triangle vertices, it is set proportional to the interior angle of that vertex
		// ==============================================================================================================================================================================================================
		glm::dvec3 XY = glm::normalize(vertices[yi] - vertices[xi]);
		glm::dvec3 YZ = glm::normalize(vertices[zi] - vertices[yi]);
		glm::dvec3 ZX = glm::normalize(vertices[xi] - vertices[zi]);
		glm::dvec3 n = glm::normalize(glm::cross(XY, YZ));

		static const double two_pi = 6.2831853071795864769253;
		double angleX = glm::acos(glm::dot(ZX, XY)); 	
		double angleY = glm::acos(glm::dot(XY, YZ)); 	
		double angleZ = two_pi - angleX - angleY; 	

		normals[xi] += angleX * n;
		normals[yi] += angleY * n;
		normals[zi] += angleZ * n;

		// ==============================================================================================================================================================================================================
		// compute volume, center of mass and second-order momenta
		// ==============================================================================================================================================================================================================
		double delta = glm::determinant(glm::dmat3(vertices[xi], vertices[yi], vertices[zi]));
		volume += delta;									// 1 / 3! is the factor
		glm::dvec3 C = X + Y + Z;							// face center, not normalized
		mass_center += delta * C;							// 1 / 4! is the factor

		// ==============================================================================================================================================================================================================
		// |xA xB xC|   |2 1 1|   |xA yA zA|
		// |yA yB yC| * |1 2 1| * |xB yB zB|
		// |zA zB zC|   |1 1 2|   |xC yC zC|
		// ==============================================================================================================================================================================================================
		double lxx = C.x * C.x + glm::dot(X, X);			// inputs to global momenta from the pyramid OXYZ
		double lxy = C.x * C.y + glm::dot(X, Y);			// has to be scaled by delta
		double lxz = C.x * C.z + glm::dot(X, Z);			// 1 / 5! is the integration factor
		double lyy = C.y * C.y + glm::dot(Y, Y);
		double lyz = C.y * C.z + glm::dot(Y, Z);
		double lzz = C.z * C.z + glm::dot(Z, Z);

		mxx += delta * lxx;									
		mxy += delta * lxy;
		mxz += delta * lxz;
		myy += delta * lyy;
		myz += delta * lyz;
		mzz += delta * lzz;
		++f;
	};

	debug_msg("Volume = %f", volume / 6.0);
	debug_msg("Center of mass = %s", glm::to_string(mass_center / 24.0).c_str());
	glm::dmat3 momentum_matrix = glm::dmat3(mxx, mxy, mxz, mxy, myy, myz, mxz, myz, mzz) / 120.0;
	debug_msg("Inertia matrix = %s", glm::to_string(momentum_matrix).c_str());

	free(pindex);
	free(tindex);


};

//=======================================================================================================================================================================================================================
// support function : direct maximum search, O(V) time
//=======================================================================================================================================================================================================================
double solid::support_bf(const glm::dvec3& direction, int& vertex_index)
{
	vertex_index = 0;
	double max_value = glm::dot(vertices[0], direction);
	for (unsigned int i = 1; i < V; ++i)
	{
		double value = glm::dot(vertices[i], direction);
		if (value > max_value)
		{
			max_value = value;
			vertex_index = i;
		};
	};
	return max_value;
};

//=======================================================================================================================================================================================================================
// support function : hill climbing method, O(log V) time
// the algorithm goes over vertices, each time finding the a vertex which lies higher than the previous one with respect to the height function h(p) = <p, direction>
// when no such vertex can be found algorithm has found the maximal support value.
// on entry vertex_index should contain a valid vertex index to start from and a triangle index that contains this initial vertex
// vertex_index = 0 and triangle_index = 0 is always a valid combination on a first run, then the values computed at the previous step can be used 
// this will speed up the routine even further
// ======================================================================================================================================================================================================================
double solid::support_hc(const glm::dvec3& direction, int& vertex_index, int& triangle_index)
{
	int previous_index = triangle_index;	
	double max_value = glm::dot(vertices[vertex_index], direction);

	//===============================================================================================================================================================================================================
	// loop over vertices adjacent to the current one
	//===============================================================================================================================================================================================================

	while(true)
	{
		debug_msg("vertex_index = %d. triangle_index = %d. previous_index = %d. max_value = %.16f.", vertex_index, triangle_index, previous_index, max_value);
		glm::ivec3 triangle = triangles[triangle_index];
		if (vertex_index == triangle.x)
		{
			debug_msg("checking vertex %d.", triangle.y);
			double value = glm::dot(vertices[triangle.y], direction);
			if (value > max_value)
			{
				debug_msg("XXXXX : The value at vertex %d is greater than the current value at vertex %d.", triangle.y, vertex_index);
				max_value = value;
				vertex_index = triangle.y;
				previous_index = triangle_index;
				debug_msg("XXXXX : previous index has changed to %d", previous_index);
				triangle_index = adjacency[triangle_index].z;
			}
			else
			{
				triangle_index = adjacency[triangle_index].y;
				if (triangle_index == previous_index) return max_value;
			}
			continue;
		};

		if (vertex_index == triangle.y)
		{
			debug_msg("checking vertex %d.", triangle.z);
			double value = glm::dot(vertices[triangle.z], direction);
			if (value > max_value)
			{
				debug_msg("YYYYY : The value at vertex %d is greater than the current value at vertex %d.", triangle.z, vertex_index);
				max_value = value;
				vertex_index = triangle.z;
				previous_index = triangle_index;
				debug_msg("YYYYY : previous index has changed to %d", previous_index);
				triangle_index = adjacency[triangle_index].x;
			}
			else
			{
				triangle_index = adjacency[triangle_index].z;
				if (triangle_index == previous_index) return max_value;
			};
			continue;
		};
		
		double value = glm::dot(vertices[triangle.x], direction);								// vertex_index == triangle.z
		debug_msg("checking vertex %d.", triangle.x);
		if (value > max_value)
		{
			debug_msg("ZZZZZ : The value at vertex %d is greater than the current value at vertex %d.", triangle.x, vertex_index);
			max_value = value;
			vertex_index = triangle.x;
			previous_index = triangle_index;
			debug_msg("ZZZZZ : previous index has changed to %d", previous_index);
			triangle_index = adjacency[triangle_index].y;
		}
		else
		{
			triangle_index = adjacency[triangle_index].x;
			if (triangle_index == previous_index) return max_value;
		};
	};
};

// ======================================================================================================================================================================================================================
// vertices, normals and triangles assumed to be filled prior to calling this function
// ======================================================================================================================================================================================================================
void solid::fill_buffers()
{
	// ==================================================================================================================================================================================================================
	// single-precision data for openGL buffers
	// ==================================================================================================================================================================================================================
	glm::vec3* vertices_f = (glm::vec3*) malloc(V * sizeof(glm::vec3));
	glm::vec3* normals_f = (glm::vec3*) malloc(V * sizeof(glm::vec3));

	for(unsigned int i = 0; i < V; ++i)
	{
		normals[i] = glm::normalize(normals[i]);
		vertices_f[i] = glm::vec3(vertices[i]);
		normals_f[i] = glm::vec3(normals[i]);
	};

	// ==================================================================================================================================================================================================================
	// fill buffers
	// ==================================================================================================================================================================================================================
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);

	glGenBuffers(1, &vbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
	glBufferData(GL_ARRAY_BUFFER, V * sizeof(glm::vec3), glm::value_ptr(vertices_f[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &nbo_id);
	glBindBuffer(GL_ARRAY_BUFFER, nbo_id);
	glBufferData(GL_ARRAY_BUFFER, V * sizeof(glm::vec3), glm::value_ptr(normals_f[0]), GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);


	glGenBuffers(1, &ibo_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, F * sizeof(glm::ivec3), glm::value_ptr(triangles[0]), GL_STATIC_DRAW);

	free(vertices_f);
	free(normals_f);
};


//=======================================================================================================================================================================================================================
// indexed render function
//=======================================================================================================================================================================================================================
void solid::render()
{
    glBindVertexArray(vao_id);
    glDrawElements(GL_TRIANGLES, 3 * F, GL_UNSIGNED_INT, 0);
};

//=======================================================================================================================================================================================================================
// destructor : frees up video memory buffers
//=======================================================================================================================================================================================================================
solid::~solid()
{
	glDeleteBuffers(1, &vbo_id);																				
	glDeleteBuffers(1, &nbo_id);
	glDeleteBuffers(1, &ibo_id);
	glDeleteVertexArrays(1, &vao_id);
};