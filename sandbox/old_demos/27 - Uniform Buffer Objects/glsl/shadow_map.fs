#version 430

in vec4 position;

layout(location = 0) out float r;
        
void main()
{
    r = length(position);
};

