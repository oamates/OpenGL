#pragma once

#include "node.hpp"
#include "../util/single_active.hpp"
#include "../types/instance_pool.hpp"

struct Camera;
struct MeshDrawer;
struct Texture2D;
struct Material;
struct Light;
struct Node;

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

    explicit Scene(std::string filepath);
    ~Scene();
        
    const std::string &GetFilepath() const;         // Gets the load scene filepath.
    const std::string &GetDirectory() const;        // Gets the loaded scene file directory.
    void Import(unsigned int flags);                // Imports the raw data from the specified scene at filepath into usable classes.
    void CleanImport(unsigned int flags);           // Reimports the scene with the specified flags. This will clear any previous data.
    void Load();                                    // Uploads to GPU the mesh and texture data
    bool IsLoaded() const;                          // Returns if this scene file is already loaded.
    void SetAsActive() override;                    // Sets this scene as active and executes all neccesary logic for changing the current active scene
};

