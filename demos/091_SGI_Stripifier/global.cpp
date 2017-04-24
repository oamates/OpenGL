
#include "global.hpp"
#include "polverts.hpp"

int ids[MAX1];
int norms[MAX1];
int *vert_norms;
int *vert_texture;

int num_faces;
ListHead **PolFaces;
ListHead **PolEdges;
ListHead *array[60];
P_FACE_ADJACENCIES face_array;  /* Pointers from face_id to face   */
ListHead **Vertices;            /* Pointers from vertex_id to face */
ListHead *strips[1];
int orient;

