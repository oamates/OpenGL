#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../util/single_active.hpp"
#include "../util/scene_importer.hpp"
#include "../types/instance_pool.hpp"

#include "node.hpp"
#include "mesh.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "texture.hpp"

struct Material;

inline std::string GetDirectoryPath(const std::string& filePath)
{
    auto pos = filePath.find_last_of("/");
    return (std::string::npos == pos) ? "" : filePath.substr(0, pos);
}

// Represents a scene composed of many cameras, meshes, lights, textures and materials. Handles importing, loading and changing scenes.
struct Scene : public BaseObject, public SingleActive<Scene>
{
    std::vector<std::shared_ptr<MeshDrawer>> meshes;
    std::vector<std::shared_ptr<Texture2D>> textures;
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<std::shared_ptr<Camera>> cameras;
    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Node> rootNode;

    std::string filepath;
    std::string directory;

    bool isLoaded;
    bool isImported;
    int activeCamera;

    explicit Scene(std::string filepath)
        : isLoaded(false), isImported(false), activeCamera(0), filepath(filepath)
        { directory = GetDirectoryPath(filepath); }

    ~Scene() {}
        
    const std::string &GetFilepath() const                                  // Gets the load scene filepath.
        { return filepath; }

    const std::string &GetDirectory() const                                 // Gets the loaded scene file directory.
        { return directory; }

    void Import(unsigned int flags)                                         // Imports the raw data from the specified scene at filepath into usable classes.
    {
        if (isImported || isLoaded)
            return;

        SceneImporter::Import(filepath, this, flags);
        isImported = true;
        isLoaded = false;
    }

    void CleanImport(unsigned int flags)                                    // Reimports the scene with the specified flags. This will clear any previous data.
    {
        Camera::ResetActive();
        ResetActive();
        meshes.clear();
        textures.clear();
        materials.clear();
        lights.clear();
        cameras.clear();
        rootNode.reset();
        isImported = false;
        isLoaded = false;
        Import(flags);
    }

    void Load()                                                             // Uploads to GPU the mesh and texture data
    {
        if (isLoaded || !isImported)
            return;

        for (auto &mesh : meshes)
            mesh->Load();

        for (auto &texture : textures)
            texture->Load(oglplus::TextureMinFilter::LinearMipmapLinear, oglplus::TextureMagFilter::Linear, oglplus::TextureWrap::Repeat, oglplus::TextureWrap::Repeat);

        if (name.empty())
            name = rootNode->name;

        isLoaded = true;
    }

    bool IsLoaded() const                                                   // Returns if this scene file is already loaded.
        { return isLoaded; }

    void SetAsActive() override                                             // Sets this scene as active and executes all neccesary logic for changing the current active scene
    {
        auto previous = Active().get();                                     // previous active scene
    
        if (previous == this)                                               // no change
            return;

        if (previous != nullptr)
        {
            previous->activeCamera = -1;                                    // save current scene active camera index
            for (size_t i = 0; i < previous->cameras.size(); i++)
                if (previous->cameras[i]->IsActive())
                    previous->activeCamera = static_cast<int>(i);
        }

        SingleActive::SetAsActive();                                        // call base method, now this instace is marked as active
        Light::ResetCollections();                                          // clean current light per type collections
    
        for (auto &light : lights)                                          // add all the current scene lights to type collection
            light->TypeCollection(light->Type());

        if (activeCamera != -1 && activeCamera < cameras.size())            // once changing scene previous active cameras from other scene shouldn't be active
            cameras[activeCamera]->SetAsActive();
        else
            Camera::ResetActive();
    }
};