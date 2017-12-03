#include "octree.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <limits>

namespace tribox 
{
    struct Vector
    {
        float x, y, z;       

        Vector(void)
        { }

        Vector(float a, float b, float c)
            : x(a), y(b), z(c)
        { }

        Vector operator - (Vector &v2)
        {
            Vector v3;
            v3.x = x - v2.x;
            v3.y = y - v2.y;
            v3.z = z - v2.z;
            return v3;
        }
    }; 

    struct Point
    {
        float x, y, z;  

        Point(void)
        { }

        Point(float a, float b, float c)
            : x(a), y(b), z(c)
        { }

        Vector operator - (Point& p2)
        {
            Vector v3;
            v3.x = x - p2.x;
            v3.y = y - p2.y;
            v3.z = z - p2.z;
            return v3;
        }
    }; 

    struct Sphere 
    {
        Point c;    // center
        float r;    // radius
    };

    float Dot(Vector v1, Vector v2)
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    float Dot(Point p1, Vector v2)
    {
        return p1.x * v2.x + p1.y * v2.y + p1.z * v2.z;
    }

    Vector Cross(Vector v1, Vector v2)
    {
        Vector v3;       
        v3.x = v1.y * v2.z - v2.y * v1.z;
        v3.y = v1.z * v2.x - v1.x * v2.z;
        v3.z = v1.x * v2.y - v1.y * v2.x;
        return v3;
    }

    Vector operator * (float scale, Vector v)
    {
        Vector v3;
        v3.x = scale * v.x;
        v3.y = scale * v.y;
        v3.z = scale * v.z;
        return v3;
    }

    Point operator + (Point p, Vector v)
    {
        Point p2;
        p2.x = p.x + v.x;
        p2.y = p.y + v.y;
        p2.z = p.z + v.z;
        return p2;
    }

    Point operator + (Point p, Point p2)
    {
        Point p3;
        p3.x = p.x + p2.x;
        p3.y = p.y + p2.y;
        p3.z = p.z + p2.z;
        return p3;
    }

    Point operator * (float scale, Point p)
    {
        Point p2;
        p2.x = scale * p.x;
        p2.y = scale * p.y;
        p2.z = scale * p.z;
        return p2;
    }


    float getmin(const std::vector<Point>& points, Vector axis)
    {
        float min = std::numeric_limits<float>::max(); 

        for (int ctr = 0; ctr < points.size(); ctr++)
        {
            float dotprod = Dot(points[ctr], axis);
            if (dotprod < min)
                min = dotprod;
        }
        return min;
    }

    float getmax(const std::vector<Point>& points, Vector axis)
    {
        float max = -std::numeric_limits<float>::max(); 

        for (int ctr = 0; ctr < points.size(); ctr++)
        {
            float dotprod = Dot(points[ctr], axis);
            if (dotprod > max)
                max = dotprod;
        }
        return max;
    }

    bool isect(const std::vector<Point> &points1, const std::vector<Point> &points2, Vector axis)
    {
        if (getmin(points1, axis) > getmax(points2, axis)) return false;
        if (getmax(points1, axis) < getmin(points2, axis)) return false;
        return true;     
    }

    bool isectboxtri(float center[3], float r[3], float triverts[3][3])
    {
        std::vector<Point> boxpoints;
        boxpoints.push_back(Point(center[0] + r[0], center[1] + r[1], center[2] + r[2]));
        boxpoints.push_back(Point(center[0] + r[0], center[1] + r[1], center[2] - r[2]));
        boxpoints.push_back(Point(center[0] + r[0], center[1] - r[1], center[2] + r[2]));
        boxpoints.push_back(Point(center[0] + r[0], center[1] - r[1], center[2] - r[2]));
        boxpoints.push_back(Point(center[0] - r[0], center[1] + r[1], center[2] + r[2]));
        boxpoints.push_back(Point(center[0] - r[0], center[1] + r[1], center[2] - r[2]));
        boxpoints.push_back(Point(center[0] - r[0], center[1] - r[1], center[2] + r[2]));
        boxpoints.push_back(Point(center[0] - r[0], center[1] - r[1], center[2] - r[2]));
     
        std::vector<Point> tripoints;
        tripoints.push_back(Point(triverts[0][0], triverts[0][1], triverts[0][2]));
        tripoints.push_back(Point(triverts[1][0], triverts[1][1], triverts[1][2]));
        tripoints.push_back(Point(triverts[2][0], triverts[2][1], triverts[2][2]));

        // test the x, y, and z axes
        if (!isect(boxpoints, tripoints, Vector(1, 0, 0))) return false;
        if (!isect(boxpoints, tripoints, Vector(0, 1, 0))) return false;
        if (!isect(boxpoints, tripoints, Vector(0, 0, 1))) return false;

        // test the triangle normal
        Vector triedge1 = tripoints[1] - tripoints[0];
        Vector triedge2 = tripoints[2] - tripoints[1];
        Vector trinormal = Cross(triedge1, triedge2);
        if (!isect(boxpoints, tripoints, trinormal)) return false;

        // test the 9 edge cross products
        Vector triedge3 = tripoints[0] - tripoints[2];

        Vector boxedge1 = Vector(1, 0, 0);
        Vector boxedge2 = Vector(0, 1, 0);
        Vector boxedge3 = Vector(0, 0, 1);

        if (!isect(boxpoints, tripoints, Cross(boxedge1, triedge1))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge1, triedge2))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge1, triedge3))) return false;

        if (!isect(boxpoints, tripoints, Cross(boxedge2, triedge1))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge2, triedge2))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge2, triedge3))) return false;

        if (!isect(boxpoints, tripoints, Cross(boxedge3, triedge1))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge3, triedge2))) return false;
        if (!isect(boxpoints, tripoints, Cross(boxedge3, triedge3))) return false;

        return true; 
    }
}

octree_t::octree_t(Scene* scene)
{
    
    // init
    m_scene = scene; // scene
	m_max_level = 1; // criteres d'arret
	m_objects_max = 1;  // criteres d'arret
	m_level = 0; // niveau actuel
	m_objects_number = m_scene->m_triangles.size() + m_scene->m_planes.size() + m_scene->m_quadrics.size(); // nombre d'objets dans la scene
	m_nb_prim_max = 0; // nombre max de primitives trouvees dans un noeud (pour le passage des donnees au glsl)
	
    // Calcul de la boite englobante de la scene
	for (int i = 0; i < m_scene->m_triangles.size(); i++)
	{
		if (i == 0)
		{
			m_xmin = m_scene->m_triangles[i].p0.x; 
			m_xmax = m_scene->m_triangles[i].p0.x; 
			m_ymin = m_scene->m_triangles[i].p0.y; 
			m_ymax = m_scene->m_triangles[i].p0.y; 
			m_zmin = m_scene->m_triangles[i].p0.z; 
			m_zmax = m_scene->m_triangles[i].p0.z; 
		}
		m_xmin = std::min(std::min(std::min(m_xmin, float(m_scene->m_triangles[i].p0.x)), float(m_scene->m_triangles[i].p1.x)), float(m_scene->m_triangles[i].p2.x));
		m_ymin = std::min(std::min(std::min(m_ymin, float(m_scene->m_triangles[i].p0.y)), float(m_scene->m_triangles[i].p1.y)), float(m_scene->m_triangles[i].p2.y));
		m_zmin = std::min(std::min(std::min(m_zmin, float(m_scene->m_triangles[i].p0.z)), float(m_scene->m_triangles[i].p1.z)), float(m_scene->m_triangles[i].p2.z));
		m_xmax = std::max(std::max(std::max(m_xmax, float(m_scene->m_triangles[i].p0.x)), float(m_scene->m_triangles[i].p1.x)), float(m_scene->m_triangles[i].p2.x));
		m_ymax = std::max(std::max(std::max(m_ymax, float(m_scene->m_triangles[i].p0.y)), float(m_scene->m_triangles[i].p1.y)), float(m_scene->m_triangles[i].p2.y));
		m_zmax = std::max(std::max(std::max(m_zmax, float(m_scene->m_triangles[i].p0.z)), float(m_scene->m_triangles[i].p1.z)), float(m_scene->m_triangles[i].p2.z));
	}
	m_sizeX = m_xmax - m_xmin;
	m_sizeY = m_ymax - m_ymin;
	m_sizeZ = m_zmax - m_zmin;
	
	// octree construction
	m_root = new node_t(); //Noeud racine
	m_nodes.push_back(*m_root);
	octree_t::build(0, m_xmin, m_ymin, m_zmin, m_xmax, m_ymax, m_zmax); // construire l'octree
	
}


// Fonction qui verifie si un triangle est contenu partiellement ou totalement dans un noeud
bool octree_t::isTriangleInNode(Triangle triangle, float xmin, float ymin, float zmin, float xmax, float ymax, float zmax){

	float zm = 0.5f * (zmin + zmax); // milieu
	float ym = 0.5f * (ymin + ymax); // milieu
	float xm = 0.5f * (xmin + xmax); // milieu
	float center[3] = {xm, ym, zm};
	float r[3] = {xmax - xmin, ymax - ymin, zmax - zmin};
	float triverts[3][3] = {
		{triangle.p0.x, triangle.p0.y, triangle.p0.z},
		{triangle.p1.x, triangle.p1.y, triangle.p1.z},
		{triangle.p2.x, triangle.p2.y, triangle.p2.z}
	};

	bool intersection = tribox::isectboxtri(center, r, triverts);
	//std::cout << intersection << std::endl;
	return intersection;
}



void octree_t::build(int cur_node_id, float xmin,float ymin,float zmin,float xmax,float ymax,float zmax)
{
	int objects_id_counter = 0;
	int local_objects_number = 0; 
	
	// Calcul du Nombre d'objets dans le noeud
	for (int i = 0; i < m_scene->m_triangles.size(); i++)
	{
		if (isTriangleInNode(m_scene->m_triangles[i], xmin, ymin, zmin, xmax, ymax, zmax))
		{
			local_objects_number++;
			m_nodes[cur_node_id].objects_id.push_back(i);
		}
	}
	
	
	// coordonnees du voxel/noeud
	m_nodes[cur_node_id].coords[0] = xmin;
	m_nodes[cur_node_id].coords[1] = ymin;
	m_nodes[cur_node_id].coords[2] = zmin;
	m_nodes[cur_node_id].coords[3] = xmax;
	m_nodes[cur_node_id].coords[4] = ymax;
	m_nodes[cur_node_id].coords[5] = zmax;

	
	if (local_objects_number > m_objects_max && m_level < m_max_level)
	{ // si le critere d'arret nest pas atteint : NOEUD INTERNE
		m_level++; // niveau de profondeur
		for (int i = 0; i < 8; i++)
		{ // creer 8 fils pour le noeud courant
			
			// ajouter le nouveau noeud enfant au vecteur de noeuds
			int child_id = m_nodes.size();
			m_nodes[cur_node_id].child[i] = child_id;
			node_t* new_node = new node_t();
			m_nodes.push_back(*new_node); 
			
			// milieu
			float zm = 0.5f * (zmin + zmax); // milieu
			float ym = 0.5f * (ymin + ymax); // milieu
			float xm = 0.5f * (xmin + xmax); // milieu
			
			// parcours des enfants
			switch (i){
				case 0: {
					//std::cout << "enfant 0" << std::endl;
					build(child_id, xmin, ymin, zmin, xm, ym, zm);
					break;
				}
				case 1: {
					//std::cout << "enfant 1" << std::endl;
					build(child_id, xmin, ymin, zm, xm, ym, zmax);
					break;
				}
				case 2: {
					//std::cout << "enfant 2" << std::endl;
					build(child_id, xmin, ym, zmin, xm, ymax, zm);
					break;
				}
				case 3: {
					//std::cout << "enfant 3" << std::endl;
					build(child_id, xmin, ym, zm, xm, ymax, zmax);
					break;
				}
				case 4: {
					//std::cout << "enfant 4" << std::endl;
					build(child_id, xm, ymin, zmin, xmax, ym, zm);
					break;
				}
				case 5: {
					//std::cout << "enfant 5" << std::endl;
					build(child_id, xm, ymin, zm, xmax, ym, zmax);
					break;
				}
				case 6: {
					//std::cout << "enfant 6" << std::endl;
					build(child_id, xm, ym, zmin, xmax, ymax, zm);
					break;
				}
				case 7: {
					//std::cout << "enfant 7" << std::endl;
					build(child_id, xm, ym, zm, xmax, ymax, zmax);
					break;
				}
			}
		}
		m_level--; // decrementer le niveau de profondeur
	}
	else { 
		//std::cout << "le noeud est terminal" << std::endl; // NOEUD TERMINAL
		for (int i = 0; i < 8; i++)
			m_nodes[cur_node_id].child[i] = -1; // enfants a -1 si le noeud est terminal
		m_nb_prim_max = std::max(m_nb_prim_max, int(m_nodes[cur_node_id].objects_id.size())); // mise a jour du nombre max de primitives trouvees dans un noeud
	}
}

octree_t::~octree_t()
{
    
}
