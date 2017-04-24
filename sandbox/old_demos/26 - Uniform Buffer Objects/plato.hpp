#ifndef _plato_included_0153674656137456743564378987346159763487516348756384765
#define _plato_included_0153674656137456743564378987346159763487516348756384765

#include <glm\glm.hpp>														

namespace plato {

    const float invroot3     = 0.5773502691896257645091487805010f;
	const float root2        = 1.4142135623730950488016887242097f;
    const float root3        = 1.7320508075688772935274463415059f;
	const float golden_ratio = 1.6180339887498948482045868343656f;

	//===================================================================================================================================================================================================================
	// texture coordinates of right polygons : triangle, square and pentagon
	//===================================================================================================================================================================================================================

	const glm::vec2 triangle[] = 
	{
		glm::vec2 (1.0f, 0.5f), 
		glm::vec2 (0.0f, 1.0f), 
		glm::vec2 (0.0f, 0.0f)
	};

	const glm::vec2 square[] = 
	{
		glm::vec2 (0.0f, 0.0f),
		glm::vec2 (1.0f, 0.0f),
		glm::vec2 (1.0f, 1.0f),
		glm::vec2 (0.0f, 1.0f)
	};

	const glm::vec2 pentagon[] =
	{
		glm::vec2 (1.0000000000000000f, 0.5000000000000000f),
		glm::vec2 (0.6545084971874737f, 0.9755282581475768f),
		glm::vec2 (0.0954915028125263f, 0.7938926261462366f),
		glm::vec2 (0.0954915028125263f, 0.2061073738537634f),
		glm::vec2 (0.6545084971874737f, 0.0244717418524232f)
	};



	//===================================================================================================================================================================================================================
	// tetrahedron VBO data 
	//===================================================================================================================================================================================================================

	namespace tetrahedron {

		const int F = 4;																									// Euler tells us, that F - E + V = 2
		const int E = 6;
		const int V = 4;
		const int mesh_size = 12;

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertex[V] = 
		{
			root2 * glm::vec3 ( 1.0f,  1.0f,  1.0f),
			root2 * glm::vec3 (-1.0f, -1.0f,  1.0f),
			root2 * glm::vec3 ( 1.0f, -1.0f, -1.0f),
			root2 * glm::vec3 (-1.0f,  1.0f, -1.0f)
		}; 

		const glm::vec3 normal[F] = 
		{
			glm::vec3 (-invroot3, -invroot3, -invroot3),
			glm::vec3 ( invroot3,  invroot3, -invroot3),
			glm::vec3 (-invroot3,  invroot3,  invroot3),
			glm::vec3 ( invroot3, -invroot3,  invroot3)
		};


		// ==============================================================================================================================================================================================================
		
		const glm::vec3 vertices[mesh_size] = 
		{
			vertex[0], vertex[1], vertex[2],																				// face (012)
			vertex[3], vertex[2], vertex[1],																				// face (321)
			vertex[2], vertex[3], vertex[0], 																				// face (230)
			vertex[1], vertex[0], vertex[3], 																				// face (103)
		};                                                                                                                  

		const glm::vec3 normals[mesh_size] = 
		{
			normal[3], normal[3], normal[3],																				// face (012)
			normal[0], normal[0], normal[0],																				// face (321)
			normal[1], normal[1], normal[1], 																				// face (230)
			normal[2], normal[2], normal[2] 																				// face (103)
		};                                                                                                                  

		const glm::vec2 uvs[mesh_size] = 
		{
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2]
		};

	}; // namespace tetrahedron                                                                                                          			


	//===================================================================================================================================================================================================================
	// cube VBO data 
	//===================================================================================================================================================================================================================

	namespace cube {                                                                                            			

		const int F = 6;
		const int E = 12;
		const int V = 8;
		const int mesh_size = 36;

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertex[V] = 
		{
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3( 1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3( 1.0f,  1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
			glm::vec3( 1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3( 1.0f,  1.0f,  1.0f)
		};

		const glm::vec3 normal[F] = 
		{ 
			glm::vec3( 1.0f,  0.0f,  0.0f),
			glm::vec3(-1.0f,  0.0f,  0.0f),
			glm::vec3( 0.0f,  1.0f,  0.0f),
			glm::vec3( 0.0f, -1.0f,  0.0f),
			glm::vec3( 0.0f,  0.0f,  1.0f),
			glm::vec3( 0.0f,  0.0f, -1.0f)
		};

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertices[mesh_size] = 
		{
			vertex[0], vertex[2], vertex[3], vertex[0], vertex[3], vertex[1],												// faces parallel to xy plane : the face [0231] and ...
			vertex[4], vertex[5], vertex[7], vertex[4], vertex[7], vertex[6],												// ... the face [4576]
			vertex[0], vertex[4], vertex[6], vertex[0], vertex[6], vertex[2],												// faces parallel to yz plane : the face [0462] and ...
			vertex[1], vertex[3], vertex[7], vertex[1], vertex[7], vertex[5],												// ... the face [1375]
			vertex[0], vertex[1], vertex[5], vertex[0], vertex[5], vertex[4],									            // faces parallel to zx plane : the face [0154] and ...
			vertex[2], vertex[6], vertex[7], vertex[2], vertex[7], vertex[3]												// ... the face [2673]
		};   

		const glm::vec3 normals[mesh_size] = 
		{
			normal[5], normal[5], normal[5], normal[5], normal[5], normal[5],
			normal[4], normal[4], normal[4], normal[4], normal[4], normal[4],
			normal[1], normal[1], normal[1], normal[1], normal[1], normal[1],
			normal[0], normal[0], normal[0], normal[0], normal[0], normal[0],
			normal[3], normal[3], normal[3], normal[3], normal[3], normal[3],
			normal[2], normal[2], normal[2], normal[2], normal[2], normal[2],
		};   
                                                                                                   			
		const glm::vec2 uvs[mesh_size] = 
		{
			square[0], square[1], square[2], square[0], square[2], square[3],
			square[0], square[1], square[2], square[0], square[2], square[3],
			square[0], square[1], square[2], square[0], square[2], square[3],
			square[0], square[1], square[2], square[0], square[2], square[3],
			square[0], square[1], square[2], square[0], square[2], square[3],
			square[0], square[1], square[2], square[0], square[2], square[3]
		};   

	}; // namespace cube 

	//===================================================================================================================================================================================================================
	// octahedron VBO data 
	//===================================================================================================================================================================================================================

	namespace octahedron {

		const int F = 8;																						
		const int E = 12;
		const int V = 6;
		const int mesh_size = 24;

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertex[V] = 
		{

			root3 * glm::vec3(-1.0f,  0.0f,  0.0f),
			root3 * glm::vec3( 1.0f,  0.0f,  0.0f),
			root3 * glm::vec3( 0.0f, -1.0f,  0.0f),
			root3 * glm::vec3( 0.0f,  1.0f,  0.0f),
			root3 * glm::vec3( 0.0f,  0.0f, -1.0f),
			root3 * glm::vec3( 0.0f,  0.0f,  1.0f)
		};

        const glm::vec3 normal[F] = 
		{
			glm::vec3 ( invroot3,  invroot3,  invroot3),
			glm::vec3 (-invroot3,  invroot3,  invroot3),
			glm::vec3 ( invroot3, -invroot3,  invroot3),
			glm::vec3 (-invroot3, -invroot3,  invroot3),
			glm::vec3 ( invroot3,  invroot3, -invroot3),
			glm::vec3 (-invroot3,  invroot3, -invroot3),
			glm::vec3 ( invroot3, -invroot3, -invroot3),
			glm::vec3 (-invroot3, -invroot3, -invroot3)
		};
		
		// ==============================================================================================================================================================================================================

		const glm::vec3 vertices[mesh_size] = 
		{
			vertex[1], vertex[3], vertex[5], 																				// faces above xy plane : (135),
			vertex[5], vertex[2], vertex[1], 																				// (521)
			vertex[5], vertex[3], vertex[0], 																				// (530)
			vertex[0], vertex[2], vertex[5], 																				// (025)
			vertex[1], vertex[2], vertex[4], 																				// faces below xy plane : (124),
			vertex[4], vertex[3], vertex[1], 																				// (431)
			vertex[0], vertex[3], vertex[4], 																				// (034)
			vertex[4], vertex[2], vertex[0] 																				// (420)
		};                                                                                                     

		const glm::vec3 normals[mesh_size] = 
		{
			normal[0], normal[0], normal[0], 
			normal[2], normal[2], normal[2], 
			normal[1], normal[1], normal[1], 
			normal[3], normal[3], normal[3], 
			normal[6], normal[6], normal[6], 
			normal[4], normal[4], normal[4], 
			normal[5], normal[5], normal[5], 
			normal[7], normal[7], normal[7], 
		};   

		const glm::vec2 uvs[mesh_size] = 
		{
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2]
		};

	}; // namespace octahedron
                                                                                                          			
	//===================================================================================================================================================================================================================
	// dodecahedron VBO data 
	//===================================================================================================================================================================================================================

	namespace dodecahedron {

		const int F = 12;
		const int E = 30;
		const int V = 20;
		const int mesh_size = 108;

		const glm::vec3 vertex[V] = 
		{

			cube::vertex[0],
			cube::vertex[1],
			cube::vertex[2],
			cube::vertex[3],
			cube::vertex[4],
			cube::vertex[5],
			cube::vertex[6],
			cube::vertex[7],
																															// pair of vertices forming pyramid on the  back xy - face of cube
			glm::vec3 (-1/golden_ratio, 0, -golden_ratio),																	// v8 (bottom)   
			glm::vec3 ( 1/golden_ratio, 0, -golden_ratio),  																// v9 (top)      
																															// pair of vertices forming pyramid on the front xy - face of cube
			glm::vec3 (-1/golden_ratio, 0,  golden_ratio),																	// v10(bottom)   						
			glm::vec3 ( 1/golden_ratio, 0,  golden_ratio),  																// v11(top)      
																															// pair of vertices forming pyramid on the  back yz - face of cube
			glm::vec3 (-golden_ratio, -1/golden_ratio, 0),																	// v12(bottom)   
			glm::vec3 (-golden_ratio,  1/golden_ratio, 0),																	// v13(top)      
																															// pair of vertices forming pyramid on the front yz - face of cube
			glm::vec3 ( golden_ratio, -1/golden_ratio, 0),																	// v14(bottom)   
			glm::vec3 ( golden_ratio,  1/golden_ratio, 0),  																// v15(top)      
																															// pair of vertices forming pyramid on the  back zx - face of cube  
			glm::vec3 (0, -golden_ratio, -1/golden_ratio),																	// v16(bottom)   
			glm::vec3 (0, -golden_ratio,  1/golden_ratio),  																// v17(top)      
																															// pair of vertices forming pyramid on the front zx - face of cube
			glm::vec3 (0,  golden_ratio, -1/golden_ratio),																	// v18(bottom)   	
			glm::vec3 (0,  golden_ratio,  1/golden_ratio)   																// v19(top)      

		};

		const glm::vec3 normal[F] = 
		{
			glm::vec3(0.0f, -0.525731, -0.850651),
			glm::vec3(0.0f, 0.525731, -0.850651),
			glm::vec3(0.0f, 0.525731, 0.850651),
			glm::vec3(0.0f, -0.525731, 0.850651),
			glm::vec3(-0.850651, 0.0f, -0.525731),
			glm::vec3(-0.850651, 0.0f, 0.525731),
			glm::vec3(0.850651, 0.0f, 0.525731),
			glm::vec3(0.850651, 0.0f, -0.525731),
			glm::vec3(-0.525731, -0.850651, 0.0f),
			glm::vec3(0.525731, -0.850651,  0.0f),
			glm::vec3(0.525731, 0.850651,   0.0f),
			glm::vec3(-0.525731, 0.850651,  0.0f)
		};

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertices[mesh_size] = 
		{
																															// pair of pentagon faces forming pyramid on the  back xy - face of cube
			vertex[ 8], vertex[ 9], vertex[ 1], vertex[ 8], vertex[ 1], vertex[16], vertex[ 8], vertex[16], vertex[ 0],		// face ( 8  9  1 16  0) and ...
			vertex[ 9], vertex[ 8], vertex[ 2], vertex[ 9], vertex[ 2], vertex[18], vertex[ 9], vertex[18], vertex[ 3],		// ...  ( 9  8  2 18  3) 
																															// pair of pentagon faces forming pyramid on the front xy - face of cube
			vertex[10], vertex[11], vertex[ 7], vertex[10], vertex[ 7], vertex[19], vertex[10], vertex[19], vertex[ 6],		// face (10 11  7 19  6) and ...
			vertex[11], vertex[10], vertex[ 4], vertex[11], vertex[ 4], vertex[17], vertex[11], vertex[17], vertex[ 5],		// ...  (11 10  4 17  5) 
																															// pair of pentagon faces forming pyramid on the  back yz - face of cube
			vertex[12], vertex[13], vertex[ 2], vertex[12], vertex[ 2], vertex[ 8], vertex[12], vertex[ 8], vertex[ 0],		// face (12 13  2  8  0) and ...
			vertex[13], vertex[12], vertex[ 4], vertex[13], vertex[ 4], vertex[10], vertex[13], vertex[10], vertex[ 6],		// ...  (13 12  4 10  6) 
																															// pair of pentagon faces forming pyramid on the front yz - face of cube
			vertex[14], vertex[15], vertex[ 7], vertex[14], vertex[ 7], vertex[11], vertex[14], vertex[11], vertex[ 5],		// face (14 15  7 11  5) and ...
			vertex[15], vertex[14], vertex[ 1], vertex[15], vertex[ 1], vertex[ 9], vertex[15], vertex[ 9], vertex[ 3],		// ...  (15 14  1  9  3) 
																															// pair of pentagon faces forming pyramid on the  back zx - face of cube
			vertex[16], vertex[17], vertex[ 4], vertex[16], vertex[ 4], vertex[12], vertex[16], vertex[12], vertex[ 0],		// face (16 17  4 12  0) and ...
			vertex[17], vertex[16], vertex[ 1], vertex[17], vertex[ 1], vertex[14], vertex[17], vertex[14], vertex[ 5],		// ...  (17 16  1 14  5) 
																															// pair of pentagon faces forming pyramid on the front zx - face of cube
			vertex[18], vertex[19], vertex[ 7], vertex[18], vertex[ 7], vertex[15], vertex[18], vertex[15], vertex[ 3],		// face (18 19  7 15  3) and ...
			vertex[19], vertex[18], vertex[ 2], vertex[19], vertex[ 2], vertex[13], vertex[19], vertex[13], vertex[ 6] 		// ...  (19 18  2 13  6) 
		};                                                                                                     

		const glm::vec3 normals[mesh_size] = 
		{
			normal[ 0], normal[ 0], normal[ 0], normal[ 0], normal[ 0], normal[ 0], normal[ 0], normal[ 0], normal[ 0],	
			normal[ 1], normal[ 1], normal[ 1], normal[ 1], normal[ 1], normal[ 1], normal[ 1], normal[ 1], normal[ 1],	
			normal[ 2], normal[ 2], normal[ 2], normal[ 2], normal[ 2], normal[ 2], normal[ 2], normal[ 2], normal[ 2],		
			normal[ 3], normal[ 3], normal[ 3], normal[ 3], normal[ 3], normal[ 3], normal[ 3], normal[ 3], normal[ 3],		
			normal[ 4], normal[ 4], normal[ 4], normal[ 4], normal[ 4], normal[ 4], normal[ 4], normal[ 4], normal[ 4],		
			normal[ 5], normal[ 5], normal[ 5], normal[ 5], normal[ 5], normal[ 5], normal[ 5], normal[ 5], normal[ 5],		
			normal[ 6], normal[ 6], normal[ 6], normal[ 6], normal[ 6], normal[ 6], normal[ 6], normal[ 6], normal[ 6],		
			normal[ 7], normal[ 7], normal[ 7], normal[ 7], normal[ 7], normal[ 7], normal[ 7], normal[ 7], normal[ 7],		
			normal[ 8], normal[ 8], normal[ 8], normal[ 8], normal[ 8], normal[ 8], normal[ 8], normal[ 8], normal[ 8],		
			normal[ 9], normal[ 9], normal[ 9], normal[ 9], normal[ 9], normal[ 9], normal[ 9], normal[ 9], normal[ 9],		
			normal[10], normal[10], normal[10], normal[10], normal[10], normal[10], normal[10], normal[10], normal[10],		
			normal[11], normal[11], normal[11], normal[11], normal[11], normal[11], normal[11], normal[11], normal[11] 		
		};                                              

		const glm::vec2 uvs[mesh_size] = 
		{
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4], 
			pentagon[0], pentagon[1], pentagon[2], pentagon[0], pentagon[2], pentagon[3], pentagon[0], pentagon[3], pentagon[4]
		};


	}; // namespace dodecahedron

	//===================================================================================================================================================================================================================
	// icosahedron VBO data 
	//===================================================================================================================================================================================================================

	namespace icosahedron {

		const int F = 20;
		const int E = 30;
		const int V = 12;
		const int mesh_size = 60;

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertex[] = 
		{

			glm::vec3 (0, -1, -golden_ratio),
			glm::vec3 (0, -1,  golden_ratio),
			glm::vec3 (0,  1, -golden_ratio),
			glm::vec3 (0,  1,  golden_ratio),
			glm::vec3 (-1, -golden_ratio, 0),
			glm::vec3 (-1,  golden_ratio, 0),
			glm::vec3 ( 1, -golden_ratio, 0),
			glm::vec3 ( 1,  golden_ratio, 0),
			glm::vec3 (-golden_ratio, 0, -1),
			glm::vec3 ( golden_ratio, 0, -1),
			glm::vec3 (-golden_ratio, 0,  1),
			glm::vec3 ( golden_ratio, 0,  1)
		};

		const glm::vec3 normal[] = 
		{
			glm::vec3(-0.356822, -0.000000, -0.934172),
			glm::vec3( 0.356822,  0.000000, -0.934172),
			glm::vec3( 0.000000, -0.934172, -0.356822),
			glm::vec3(-0.577350, -0.577350, -0.577350),
			glm::vec3( 0.577350, -0.577350, -0.577350),
			glm::vec3(-0.356822,  0.000000,  0.934172),
			glm::vec3( 0.356822,  0.000000,  0.934172),
			glm::vec3( 0.000000, -0.934172,  0.356822),
			glm::vec3(-0.577350, -0.577350,  0.577350),
			glm::vec3( 0.577350, -0.577350,  0.577350),
			glm::vec3( 0.000000,  0.934172, -0.356822),
			glm::vec3(-0.577350,  0.577350, -0.577350),
			glm::vec3( 0.577350,  0.577350, -0.577350),
			glm::vec3(-0.000000,  0.934172,  0.356822),
			glm::vec3(-0.577350,  0.577350,  0.577350),
			glm::vec3( 0.577350,  0.577350,  0.577350),
			glm::vec3(-0.934172, -0.356822,  0.000000),
			glm::vec3(-0.934172,  0.356822,  0.000000),
			glm::vec3( 0.934172, -0.356822,  0.000000),
			glm::vec3( 0.934172,  0.356822, -0.000000)
		};

		// ==============================================================================================================================================================================================================

		const glm::vec3 vertices[mesh_size] = 
		{

			vertex[ 2], vertex[ 0], vertex[ 8],
			vertex[ 0], vertex[ 2], vertex[ 9],
			vertex[ 4], vertex[ 0], vertex[ 6],
			vertex[ 0], vertex[ 4], vertex[ 8],
			vertex[ 6], vertex[ 0], vertex[ 9],
			vertex[ 1], vertex[ 3], vertex[10],
			vertex[ 3], vertex[ 1], vertex[11],
			vertex[ 1], vertex[ 4], vertex[ 6],
			vertex[ 4], vertex[ 1], vertex[10],
			vertex[ 1], vertex[ 6], vertex[11],
			vertex[ 2], vertex[ 5], vertex[ 7],
			vertex[ 5], vertex[ 2], vertex[ 8],
			vertex[ 2], vertex[ 7], vertex[ 9],
			vertex[ 5], vertex[ 3], vertex[ 7],
			vertex[ 3], vertex[ 5], vertex[10],
			vertex[ 7], vertex[ 3], vertex[11],
			vertex[ 8], vertex[ 4], vertex[10],
			vertex[ 5], vertex[ 8], vertex[10],
			vertex[ 6], vertex[ 9], vertex[11],
			vertex[ 9], vertex[ 7], vertex[11]
		};

		const glm::vec3 normals[mesh_size] = 
		{
			normal[ 0], normal[ 0], normal[ 0],
			normal[ 1], normal[ 1], normal[ 1],
			normal[ 2], normal[ 2], normal[ 2],
			normal[ 3], normal[ 3], normal[ 3],
			normal[ 4], normal[ 4], normal[ 4],
			normal[ 5], normal[ 5], normal[ 5],
			normal[ 6], normal[ 6], normal[ 6],
			normal[ 7], normal[ 7], normal[ 7],
			normal[ 8], normal[ 8], normal[ 8],
			normal[ 9], normal[ 9], normal[ 9],
			normal[10], normal[10], normal[10],
			normal[11], normal[11], normal[11],
			normal[12], normal[12], normal[12],
			normal[13], normal[13], normal[13],
			normal[14], normal[14], normal[14],
			normal[15], normal[15], normal[15],
			normal[16], normal[16], normal[16],
			normal[17], normal[17], normal[17],
			normal[18], normal[18], normal[18],
			normal[19], normal[19], normal[19]
		};                                              

		const glm::vec2 uvs[mesh_size] = 
		{
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2],
			triangle[0], triangle[1], triangle[2]
		};

	}; // namespace icosahedron

}; // namespace plato

#endif	// _plato_included_0153674656137456743564378987346159763487516348756384765