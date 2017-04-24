#version 430

smooth in float z;

layout(location = 0) out float cube_dist;
        
void main()
{
    cube_dist = z;
};

