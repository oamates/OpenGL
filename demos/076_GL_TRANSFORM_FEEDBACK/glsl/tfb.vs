#version 330 core

layout(location = 0) in float inValue;

out float geoValue;

void main()
{
    geoValue = sqrt(inValue);
}

