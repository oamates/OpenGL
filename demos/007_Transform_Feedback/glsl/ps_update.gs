#version 330 core

layout(points) in;
layout(points) out;
layout(max_vertices = 30) out;

in float Type0[];
in vec3 Position0[];
in vec3 Velocity0[];
in float Age0[];

out float Type1;
out vec3 Position1;
out vec3 Velocity1;
out float Age1;

uniform sampler1D random_texture;
uniform float dt;
uniform float time;

const float gLauncherLifetime = 100.0f;
const float gShellLifetime = 10000.0f;
const float gSecondaryShellLifetime = 2500.0f;
                                                                                    
#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f
                                                                                    
vec3 GetRandomDir(float uv)
{
    return texture(random_texture, uv).xyz - vec3(0.5f);
}
                                                                                    
void main()                                                                         
{                                                                                   
    float Age = Age0[0] + dt;                                         
                                                                                    
    if (Type0[0] == PARTICLE_TYPE_LAUNCHER)
    {
        if (Age >= gLauncherLifetime)
        {
            Type1 = PARTICLE_TYPE_SHELL;                                            
            Position1 = Position0[0];                                               
            vec3 Dir = GetRandomDir(time / 1000.0);                                  
            Dir.y = max(Dir.y, 0.5);                                                
            Velocity1 = normalize(Dir) / 20.0;                                      
            Age1 = 0.0;                                                             
            EmitVertex();                                                           
            EndPrimitive();                                                         
            Age = 0.0;                                                              
        }                                                                           
                                                                                    
        Type1 = PARTICLE_TYPE_LAUNCHER;                                             
        Position1 = Position0[0];                                                   
        Velocity1 = Velocity0[0];                                                   
        Age1 = Age;                                                                 
        EmitVertex();                                                               
        EndPrimitive();                                                             
    }                                                                               
    else
    {                                                                          
        float DeltaTimeSecs = dt / 1000.0f;
        float t1 = Age0[0] / 1000.0;
        float t2 = Age / 1000.0;
        vec3 DeltaP = DeltaTimeSecs * Velocity0[0];
        vec3 DeltaV = vec3(DeltaTimeSecs) * (0.0, -9.81, 0.0);
                                                                                    
        if (Type0[0] == PARTICLE_TYPE_SHELL)
        {
	        if (Age < gShellLifetime)
            {
	            Type1 = PARTICLE_TYPE_SHELL;
	            Position1 = Position0[0] + DeltaP;
	            Velocity1 = Velocity0[0] + DeltaV;
	            Age1 = Age;
	            EmitVertex();
	            EndPrimitive();
	        }
            else
            {
                for (int i = 0 ; i < 10 ; i++)
                {
                    Type1 = PARTICLE_TYPE_SECONDARY_SHELL;
                    Position1 = Position0[0];
                    vec3 Dir = GetRandomDir((time + i) / 1000.0);
                    Velocity1 = normalize(Dir) / 20.0;
                    Age1 = 0.0f;
                    EmitVertex();
                    EndPrimitive();
                }
            }
        }
        else
        {
            if (Age < gSecondaryShellLifetime)
            {
                Type1 = PARTICLE_TYPE_SECONDARY_SHELL;
                Position1 = Position0[0] + DeltaP;
                Velocity1 = Velocity0[0] + DeltaV;
                Age1 = Age;
                EmitVertex();
                EndPrimitive();
            }
        }
    }
}
