#pragma once

#include "../types/scene_object.hpp"
#include "../types/instance_pool.hpp"

#include <glm/detail/type_vec3.hpp>
#include <vector>

// Holds the parameters that describe a scene light source. Supports for three different types of light sources: 
// point, directional and spot. See SceneObject
struct Light : public SceneObject, public InstancePool<Light>
{
    // Describes the light falloff behavior. Meaning its intensity depending on the distance of the lighten fragment and the
    // light source. This being f = 1 / (1 + c + (l * d) + (q * d * d)) where f is the falloff, c the Constant value, 
    // l the Linear value, q the Quadratic value and d the distance.
    struct Attenuation
    {
        float constant;
        float linear;
        float quadratic;

        Attenuation() : constant(1.0f), linear(0.2f), quadratic(0.08f) {};
        ~Attenuation() {};

        void Linear(const float &val)
            { linear = val; }
        void Quadratic(const float &val)
            { quadratic = val; }    
        void Constant(const float &val)
            { constant = val; }

        const float &Linear() const
            { return linear; }
        const float &Quadratic() const
            { return quadratic; }
        const float &Constant() const
            { return constant; }
    };

    enum LightType
    {
        Directional,
        Point,
        Spot
    };

    static std::vector<Light *> directionals;
    static std::vector<Light *> points;
    static std::vector<Light *> spots;
    int collectionIndex;

    float angleInnerCone;
    float angleOuterCone;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 intensity;                                    // The intensity per light component x -- ambient, y -- diffuse, z -- specular
    LightType lightType;

    static const unsigned int DirectionalsLimit;
    static const unsigned int PointsLimit;
    static const unsigned int SpotsLimit;

    Light();
    virtual ~Light();

    void AngleInnerCone(const float &val);                      // For a spot light the angle of the inner circle given in radians
    void AngleOuterCone(const float &val);                      // For a spot light the angle of the outer cone or basically the extent of the light, give in radians.
    void Ambient(const glm::vec3 &val);                         // Light ambient color component.
    void Diffuse(const glm::vec3 &val);                         // Light diffuse color component.
    void Specular(const glm::vec3 &val);                        // Light specular color component.
    void Intensities(const glm::vec3 &val);                     // Intensities of the ambient, diffuse and specular components as x, y, and z respectively

    float AngleInnerCone() const;
    float AngleOuterCone() const;

    const glm::vec3 &Ambient() const;
    const glm::vec3 &Diffuse() const;
    const glm::vec3 &Specular() const;
    const glm::vec3 &Intensities() const;
    const glm::vec3 &Direction() const;

    Attenuation attenuation;                                    // The attenuation structure describes the attenuation falloff function for spot and point lights
    LightType Type() const;                                     // Returns the light's type, directional, point or spot

    // Changes the light type and also adds it to one of of the light static vectors storing lights references per type of light.
    // If set to true it will add the light to a collection vector without checking for previous addition.
    // This is useful if ResetCollections has been called previously and there is a need to re-add light's to their type static collection.
    void TypeCollection(LightType val);

    static void ResetCollections();                             // Cleans the light type static collection vectors.
    static const std::vector<Light*>& Directionals();           // Returns the collection of directional lights
    static const std::vector<Light*>& Points();                 // Returns the collection of point lights
    static const std::vector<Light*>& Spots();                  // Returns the collection of spot lights
};