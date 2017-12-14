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

/// Represents a scene composed of many cameras, meshes, lights,
/// textures and materials. Handles importing, loading and changing scenes.
struct Scene : public BaseObject, public SingleActive<Scene>
{
        std::vector<std::shared_ptr<MeshDrawer>> meshes;
        std::vector<std::shared_ptr<Texture2D>> textures;
        std::vector<std::shared_ptr<Material>> materials;
        std::vector<std::shared_ptr<Camera>> cameras;
        std::vector<std::shared_ptr<Light>> lights;
        std::shared_ptr<Node> rootNode;

        /// Gets the load scene filepath.
        const std::string &GetFilepath() const;
        /// Gets the loaded scene file directory.
        const std::string &GetDirectory() const;
        /// Imports the raw data from the specified scene at filepath into usable classes.
        void Import(unsigned int flags);
        /// Reimports the scene with the specified flags.
        /// This will clear any previous data.
        void CleanImport(unsigned int flags);
        /// Uploads to GPU the mesh and texture data
        void Load();
        /// Returns if this scene file is already loaded.
        bool IsLoaded() const;
        /// Sets this scene as active and executes all neccesary
        /// logic for changing the current active scene
        void SetAsActive() override;

        explicit Scene(std::string filepath);
        ~Scene();

		std::string filepath;
        std::string directory;

        bool isLoaded;
        bool isImported;
        int activeCamera;

        friend struct SceneImporter;
};

