
#include <glm/gtx/string_cast.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>

#include "makelevelset3.hpp"
#include "log.hpp"

int main(int argc, char* argv[])
{
  
    if(argc != 4)
    {
        printf("SDFGen - A utility for converting closed oriented triangle meshes into grid-based signed distance fields.\n");
        printf("\nThe output file format is:");
        printf("<ni> <nj> <nk>\n");
        printf("<origin_x> <origin_y> <origin_z>\n");
        printf("<dx>\n");
        printf("<value_1> <value_2> <value_3> [...]\n\n");
        printf("(ni, nj, nk) are the integer dimensions of the resulting distance field.\n");
        printf("(origin_x,origin_y,origin_z) is the 3D position of the grid origin.\n");
        printf("<dx> is the grid spacing.\n\n");
        printf("<value_n> are the signed distance data values, in ascending order of i, then j, then k.\n");
        printf("The output filename will match that of the input, with the OBJ suffix replaced with SDF.\n\n");
        printf("Usage: SDFGen <filename> <dx> <padding>\n\n");
        printf("Where:\n");
        printf("\t<filename> specifies a Wavefront OBJ (text) file representing a *triangle* mesh (no quad or poly meshes allowed). File must use the suffix \".obj\".\n");
        printf("\t<dx> specifies the length of grid cell in the resulting distance field.\n");
        printf("\t<padding> specifies the number of cells worth of padding between the object bound box and the boundary of the distance field grid. Minimum is 1.\n\n");
        exit(-1);
    }

    std::string filename(argv[1]);
    if(filename.size() < 5 || filename.substr(filename.size() - 4) != std::string(".obj"))
    {
        printf("Error: Expected OBJ file with filename of the form <name>.obj.\n");
        exit(-1);
    }

    std::stringstream arg2(argv[2]);
    float dx;
    arg2 >> dx;
  
    std::stringstream arg3(argv[3]);
    int padding;
    arg3 >> padding;

    if(padding < 1)
        padding = 1;
    
    // start with a massive inside out bound box.
    glm::vec3 min_box( std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()), 
              max_box(-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max(),-std::numeric_limits<float>::max());
  
    std::cout << "Reading data.\n";

    std::ifstream infile(argv[1]);
    if(!infile)
    {
        printf("Failed to open file. Terminating.\n");
        exit(-1);
    }

    int ignored_lines = 0;
    std::string line;
    std::vector<glm::vec3> vertList;
    std::vector<glm::uvec3> faceList;
    while(!infile.eof())
    {
        std::getline(infile, line);

        //.obj files sometimes contain vertex normals indicated by "vn"
        if(line.substr(0, 1) == std::string("v") && line.substr(0, 2) != std::string("vn"))
        {
            std::stringstream data(line);
            char c;
            glm::vec3 point;
            data >> c >> point[0] >> point[1] >> point[2];
            vertList.push_back(point);
            min_box = glm::min(min_box, point);
            max_box = glm::max(max_box, point);
        }
        else if(line.substr(0, 1) == std::string("f"))
        {
            std::stringstream data(line);
            char c;
            int v0, v1, v2;
            data >> c >> v0 >> v1 >> v2;
            faceList.push_back(glm::uvec3(v0 - 1, v1 - 1, v2 - 1));
        }
        else
            ++ignored_lines; 
    }
    infile.close();
  
    if(ignored_lines > 0)
        debug_msg("Warning: %u lines were ignored since they did not contain faces or vertices.", ignored_lines);

    debug_msg("Read in %u vertices and %u faces.", (unsigned int) vertList.size(), (unsigned int) faceList.size());

    // Add padding around the box.
    glm::vec3 unit = glm::vec3(1.0f);
    min_box -= padding * dx * unit;
    max_box += padding * dx * unit;
    glm::uvec3 sizes = glm::uvec3((max_box - min_box) / dx);
  
    debug_msg("Bound box: (%s) to (%s) :: dimensions :: %s", glm::to_string(min_box).c_str(), glm::to_string(max_box).c_str(), glm::to_string(sizes).c_str());

    debug_msg("Computing signed distance field.");
    Array3f phi_grid;

    make_level_set3(faceList, vertList, min_box, dx, sizes[0], sizes[1], sizes[2], phi_grid);

    std::string outname = filename.substr(0, filename.size() - 4) + std::string(".sdf");
    debug_msg("Writing results to: %s", outname.c_str());
    
    std::ofstream outfile(outname.c_str());
    outfile << phi_grid.ni << " " << phi_grid.nj << " " << phi_grid.nk << std::endl;
    outfile << min_box[0] << " " << min_box[1] << " " << min_box[2] << std::endl;
    outfile << dx << std::endl;

    for(unsigned int i = 0; i < phi_grid.a.size(); ++i)
    {
        outfile << phi_grid.a[i] << std::endl;
    }
    outfile.close();

    debug_msg("Processing complete.");
    return 0;
}
