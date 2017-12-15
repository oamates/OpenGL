#include "scene.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "mesh.hpp"
#include "light.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "../util/scene_importer.hpp"

std::vector<Light*> Light::directionals;
std::vector<Light*> Light::points;
std::vector<Light*> Light::spots;

void Scene::SetAsActive()
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

inline std::string GetDirectoryPath(const std::string &filePath)
{
    auto pos = filePath.find_last_of("/");
    return (std::string::npos == pos) ? "" : filePath.substr(0, pos);
}


Scene::Scene(std::string filepath)
    : isLoaded(false), isImported(false), activeCamera(0)
{
    this->filepath = filepath;
    this->directory = GetDirectoryPath(filepath);
}

Scene::~Scene()
{
}

const std::string &Scene::GetFilepath() const
    { return filepath; }

const std::string &Scene::GetDirectory() const
    { return directory; }

void Scene::Import(unsigned int flags)
{
    if (isImported || isLoaded)
        return;

    SceneImporter::Import(filepath, this, flags);
    isImported = true;
    isLoaded = false;
}

void Scene::CleanImport(unsigned int flags)
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

void Scene::Load()
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

bool Scene::IsLoaded() const
    { return isLoaded; }
