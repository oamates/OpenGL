#pragma once

#include <vector>

#include <glm/detail/type_vec3.hpp>
#include <glm/detail/func_common.hpp>
#include <glm/detail/type_vec3.hpp>

#include "../types/scene_object.hpp"
#include "../types/instance_pool.hpp"

#include "camera.hpp"

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

    static std::vector<Light*> directionals;
    static std::vector<Light*> points;
    static std::vector<Light*> spots;
    int collectionIndex;

    float angleInnerCone;
    float angleOuterCone;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 intensity;                                    // The intensity per light component x -- ambient, y -- diffuse, z -- specular
    LightType lightType;

    static const unsigned int DirectionalsLimit = 3;
    static const unsigned int PointsLimit = 6;
    static const unsigned int SpotsLimit = 6;

    Light() : lightType(Directional)
    {
        name = "default-light";
        angleInnerCone = glm::radians(25.0f);
        angleOuterCone = glm::radians(30.0f);
        ambient = glm::vec3(0.0f);
        diffuse = specular = intensity = glm::vec3(1.0f);
        transform.Rotation(radians(glm::vec3(130.0f, -30.0f, 0.0f)));
        collectionIndex = -1;                               // indicates this light hasn't been added to any collection
        TypeCollection(lightType);                          // add to type collection
    }

    virtual ~Light()
    {
        if (collectionIndex < 0) return;

        // delete self from type collection
        auto UpdateIndex = [=](std::vector<Light*> &lights)
        {
            if (lights.size() == 0)
                return;
            lights.erase(lights.begin() + this->collectionIndex);
            for (auto i = 0; i < lights.size(); i++)
                lights[i]->collectionIndex = i;
        };

        switch(lightType)                                       // update indices
        {
            case Directional: UpdateIndex(directionals); break;
            case Point: UpdateIndex(points);; break;
            case Spot: UpdateIndex(spots); break;
            default: break;
        }
    }

    void AngleInnerCone(const float &val)                       // For a spot light the angle of the inner circle given in radians
        { angleInnerCone = glm::clamp(val, 0.0f, glm::pi<float>()); }

    void AngleOuterCone(const float &val)                       // For a spot light the angle of the outer cone or basically the extent of the light, give in radians.
    {
        angleOuterCone = glm::clamp(val, 0.0f, glm::pi<float>());;
        angleInnerCone = glm::min(angleInnerCone, angleOuterCone);
    }

    void Ambient(const glm::vec3 &val)                          // Light ambient color component.
        { ambient = max(val, glm::vec3(0.0f)); }

    void Diffuse(const glm::vec3 &val)                          // Light diffuse color component.
        { diffuse = max(val, glm::vec3(0.0f)); }

    void Specular(const glm::vec3 &val)                         // Light specular color component.
        { specular = max(val, glm::vec3(0.0f)); }

    void Intensities(const glm::vec3 &val)                      // Intensities of the ambient, diffuse and specular components as x, y, and z respectively
        { intensity = max(val, glm::vec3(0.0f)); }

    float AngleInnerCone() const
        { return angleInnerCone; }
    float AngleOuterCone() const
        { return angleOuterCone; }

    const glm::vec3 &Ambient() const
        { return ambient; }
    const glm::vec3 &Diffuse() const
        { return diffuse; }
    const glm::vec3 &Specular() const
        { return specular; }
    const glm::vec3 &Intensities() const
        { return intensity; }
    const glm::vec3 &Direction() const
        { return transform.Forward(); }    

    Attenuation attenuation;                                    // The attenuation structure describes the attenuation falloff function for spot and point lights

    LightType Type() const                                      // Returns the light's type, directional, point or spot
        { return lightType; }

    // Changes the light type and also adds it to one of of the light static vectors storing lights references per type of light.
    // If set to true it will add the light to a collection vector without checking for previous addition.
    // This is useful if ResetCollections has been called previously and there is a need to re-add light's to their type static collection.
    void TypeCollection(LightType val)
    {
        if (val == lightType && collectionIndex >= 0)                           // no change
            return;
    
        if (collectionIndex >= 0)                                               // this light is stored in another collection
        {
            switch (lightType)
            {
                case Directional:
                    if (collectionIndex >= directionals.size())
                        break;
                    directionals.erase(directionals.begin() + collectionIndex);
                    break;
                case Point:
                    if (collectionIndex >= points.size())
                        break;
                    points.erase(points.begin() + collectionIndex);
                    break;
                case Spot:
                    if (collectionIndex >= spots.size())
                        break;
                    spots.erase(spots.begin() + collectionIndex);
                    break;
                default:
                    break;
            }
        }

        switch (val)                                                            // we are changing our type
        {
            case Directional:
                if (directionals.size() == DirectionalsLimit)
                    return;
                collectionIndex = static_cast<int>(directionals.size());
                directionals.push_back(this);
                break;
            case Point:
                if (points.size() == PointsLimit)
                    return;
                collectionIndex = static_cast<int>(points.size());
                points.push_back(this);
                break;
            case Spot:
                if (spots.size() == SpotsLimit)
                    return;
                collectionIndex = static_cast<int>(spots.size());
                spots.push_back(this);
                break;
            default:
                break;
        }
        lightType = val;
    }

    static void ResetCollections()                              // Cleans the light type static collection vectors.
    {
        for (auto& l : instances)                               // reset collection index so type collections get properly filled
            l->collectionIndex = -1;
        directionals.clear();
        points.clear();
        spots.clear();
    }


    static const std::vector<Light*>& Directionals()            // Returns the collection of directional lights
        { return directionals; }
    static const std::vector<Light*>& Points()                  // Returns the collection of point lights
        { return points; }
    static const std::vector<Light*>& Spots()                   // Returns the collection of spot lights
        { return spots; }
};