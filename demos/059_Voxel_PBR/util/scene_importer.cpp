#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/quaternion.hpp>

#include "scene_importer.hpp"

#include "texture_importer.hpp"
#include "../scene/scene.hpp"
#include "../scene/material.hpp"
#include "../scene/mesh.hpp"
#include "../scene/camera.hpp"
#include "../scene/light.hpp"

#include "vertex.hpp"


bool SceneImporter::Import(const std::string &filepath, Scene * scene, unsigned flags)
{
    Assimp::Importer importer;
    const aiScene * mScene = importer.ReadFile(filepath, flags);

    if (!mScene)
        throw std::runtime_error(importer.GetErrorString());

    if (mScene->HasMaterials())
    {
        scene->materials.clear();
        for (unsigned int i = 0; i < mScene->mNumMaterials; i++)                        // process material properties
        {
            auto newMaterial = std::make_shared<Material>();
            ImportMaterial(mScene->mMaterials[i], *newMaterial);
            scene->materials.push_back(newMaterial);
        }
        scene->textures.clear();
        for (unsigned int i = 0; i < mScene->mNumMaterials; i++)                        // import per material and scene, textures
            ImportMaterialTextures(scene, mScene->mMaterials[i], *scene->materials[i]);
    }

    if (mScene->HasMeshes())
    {
        scene->meshes.clear();
        for (unsigned int i = 0; i < mScene->mNumMeshes; i++)
        {
            auto newMesh = std::make_shared<MeshDrawer>();
            ImportMesh(mScene->mMeshes[i], *newMesh);
            newMesh->material = scene->materials[mScene->mMeshes[i]->mMaterialIndex];   // material assigned to mesh
            scene->meshes.push_back(move(newMesh));
        }
    }

    if (mScene->mRootNode != nullptr)
    {
        if (!scene->rootNode) scene->rootNode = std::make_shared<Node>();
        ProcessNodes(scene, mScene->mRootNode, *scene->rootNode);
    }

    if (mScene->HasCameras())
    {
        scene->cameras.clear();
        for (unsigned int i = 0; i < mScene->mNumCameras; i++)
        {
            auto newCamera = std::make_shared<Camera>();
            ImportCamera(mScene->mCameras[i], *newCamera);
            scene->cameras.push_back(newCamera);
        }
    }
    else
        scene->cameras.push_back(std::make_shared<Camera>());                           // add one default camera

    if (mScene->HasLights())
    {
        scene->lights.clear();
        for (unsigned int i = 0; i < mScene->mNumLights; i++)
        {
            auto newLight = std::make_shared<Light>();
            ImportLight(mScene->mLights[i], *newLight);
            scene->lights.push_back(newLight);
        }
    }
    else
        scene->lights.push_back(std::make_shared<Light>());                             // add one default light

    importer.FreeScene();
    return true;
}

void SceneImporter::ImportLight(aiLight* mLight, Light& light)
{
    light.name = mLight->mName.length > 0 ? mLight->mName.C_Str() : light.name;
    light.Ambient(glm::vec3(mLight->mColorAmbient.r, mLight->mColorAmbient.g, mLight->mColorAmbient.b) / 255.0f);
    light.Diffuse(glm::vec3(mLight->mColorDiffuse.r, mLight->mColorDiffuse.g, mLight->mColorDiffuse.b) / 255.0f);
    light.Specular(glm::vec3(mLight->mColorSpecular.r, mLight->mColorSpecular.g, mLight->mColorSpecular.b) / 255.0f);
    light.transform.Forward(glm::vec3(mLight->mDirection.x, mLight->mDirection.y, mLight->mDirection.z));
    light.transform.Position(glm::vec3(mLight->mPosition.x, mLight->mPosition.y, mLight->mPosition.z));
    light.TypeCollection(  mLight->mType == aiLightSource_POINT       ? Light::Point
                         : mLight->mType == aiLightSource_DIRECTIONAL ? Light::Directional
                         : mLight->mType == aiLightSource_SPOT        ? Light::Spot : Light::Point );
    light.AngleInnerCone(mLight->mAngleInnerCone);
    light.AngleOuterCone(mLight->mAngleOuterCone);
    light.attenuation.Constant(mLight->mAttenuationConstant);
    light.attenuation.Linear(mLight->mAttenuationLinear);
    light.attenuation.Quadratic(mLight->mAttenuationQuadratic);
}

void SceneImporter::ImportCamera(aiCamera * mCam, Camera &camera)
{
    camera.name = mCam->mName.length > 0 ? mCam->mName.C_Str() : camera.name;
    camera.AspectRatio(mCam->mAspect);
    camera.ClipPlaneFar(mCam->mClipPlaneFar);
    camera.ClipPlaneNear(mCam->mClipPlaneNear);
    camera.FieldOfView(mCam->mHorizontalFOV);
    auto lookat = glm::vec3(mCam->mLookAt.x, mCam->mLookAt.y, mCam->mLookAt.z);
    auto pos = glm::vec3(mCam->mPosition.x, mCam->mPosition.y, mCam->mPosition.z);
    auto up = glm::vec3(mCam->mUp.x, mCam->mUp.y, mCam->mUp.z);
    auto fwd = normalize(lookat - pos);
    camera.transform.Position(pos);
    camera.transform.Forward(fwd);
    camera.transform.Up(up);    
    camera.SetAsActive();                                                               // latest created always as active
}

void SceneImporter::ImportMaterial(aiMaterial* mMaterial, Material& material)
{
    aiString matName;                                                                   // assimp scene material name extract
    mMaterial->Get(AI_MATKEY_NAME, matName);
    material.name = matName.length > 0 ? matName.C_Str() : material.name;
    float refracti, shininess, shinStrength;    
    mMaterial->Get(AI_MATKEY_REFRACTI, refracti);                                       // material factors
    mMaterial->Get(AI_MATKEY_SHININESS, shininess);
    mMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shinStrength);
    material.refraction_index = refracti;
    material.shininess_exponent = ((log2(shininess) + 1.0f) / 11.0f);
    material.shininess_strength = shinStrength;
    mMaterial->Get(AI_MATKEY_COLOR_AMBIENT, material.ambient);
    mMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, material.diffuse);
    mMaterial->Get(AI_MATKEY_COLOR_SPECULAR, material.specular);
    mMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, material.emissive);
    mMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, material.transparent);
}

void SceneImporter::ImportMesh(aiMesh * mMesh, Mesh &mesh)
{
    mesh.name = mMesh->mName.length > 0 ? mMesh->mName.C_Str() : mesh.name;

    if (mMesh->mNumVertices > 0)
    {
        for (unsigned int i = 0; i < mMesh->mNumVertices; i++)
        {
            vertex_pft3_t vertex;                                                                   // store mesh data
            vertex.position = glm::vec3(mMesh->mVertices[i].x, mMesh->mVertices[i].y, mMesh->mVertices[i].z);
            vertex.normal = glm::vec3(mMesh->mNormals[i].x, mMesh->mNormals[i].y, mMesh->mNormals[i].z);
            vertex.uvw = (mMesh->HasTextureCoords(0)) ? glm::vec3(mMesh->mTextureCoords[0][i].x, mMesh->mTextureCoords[0][i].y, mMesh->mTextureCoords[0][i].z) : 
                                                        glm::vec3(0.0f);

            if (mMesh->HasTangentsAndBitangents())
            {
                vertex.tangent_x = glm::vec3(mMesh->mTangents[i].x, mMesh->mTangents[i].y, mMesh->mTangents[i].z);
                vertex.tangent_y = glm::vec3(mMesh->mBitangents[i].x, mMesh->mBitangents[i].y, mMesh->mBitangents[i].z);
            }
            else
                vertex.tangent_x = vertex.tangent_y = glm::vec3(0.0f);
            
            mesh.boundaries.MinPoint(vertex.position);                                              // update boundaries with current position
            mesh.boundaries.MaxPoint(vertex.position);
            vertex.normal = glm::normalize(vertex.normal);                                          // gram-schmidt orthonormalization
            vertex.tangent_x = vertex.tangent_x - glm::dot(vertex.tangent_x, vertex.normal) * vertex.normal;
            vertex.tangent_x = glm::normalize(vertex.tangent_x);
            vertex.tangent_y = vertex.tangent_y - glm::dot(vertex.tangent_y, vertex.normal) * vertex.normal;
            vertex.tangent_y = glm::normalize(vertex.tangent_y);
            if (glm::dot(glm::cross(vertex.normal, vertex.tangent_x), vertex.tangent_y) < 0.0f)     // secure handedness
                vertex.tangent_x = -vertex.tangent_x;
            mesh.vertices.push_back(vertex);                                                        // new vertex to raw mesh data
        }
    }

    for (unsigned int i = 0; i < mMesh->mNumFaces; i++)
    {
        mesh.indices.push_back(mMesh->mFaces[i].mIndices[0]);
        mesh.indices.push_back(mMesh->mFaces[i].mIndices[1]);
        mesh.indices.push_back(mMesh->mFaces[i].mIndices[2]);
    }
}

void SceneImporter::ProcessNodes(Scene * scene, aiNode * mNode, Node &node)
{
    node.name = mNode->mName.length > 0 ? mNode->mName.C_Str() : node.name;
    
    for (unsigned int i = 0; i < mNode->mNumMeshes; i++)                                    // meshes associated with this node
    {
        node.meshes.push_back(scene->meshes[mNode->mMeshes[i]]);                            // insert after same name   
        node.boundaries.MinPoint(scene->meshes[mNode->mMeshes[i]]->boundaries.MinPoint());  // node boundaries based on mesh boundaries
        node.boundaries.MaxPoint(scene->meshes[mNode->mMeshes[i]]->boundaries.MaxPoint());
    }
    
    for (unsigned int i = 0; i < mNode->mNumChildren; i++)                                  // push childrens in hierachy
    {
        node.nodes.push_back(std::make_shared<Node>());                                     // create children
        ProcessNodes(scene, mNode->mChildren[i], *node.nodes.back());
        node.boundaries.MinPoint(node.nodes.back()->boundaries.MinPoint());                 // node boundaries based on children node boundaries
        node.boundaries.MaxPoint(node.nodes.back()->boundaries.MaxPoint());
    }

    aiVector3D pos; aiVector3D sca; aiQuaternion rot;                                       // transformation matrix decomposition using assimp implementation
    mNode->mTransformation.Decompose(sca, rot, pos);
    node.transform.Position(glm::vec3(pos.x, pos.y, pos.z));
    node.transform.Scale(glm::vec3(sca.x, sca.y, sca.z));
    node.transform.Rotation(glm::quat(rot.w, rot.x, rot.y, rot.z));
    node.BuildDrawList();                                                                   // build per node draw lists from recursive draw useful for easier batching
}

inline std::string GetFileExtension(const std::string &sFilepath)
{
    auto result = sFilepath.substr(sFilepath.find_last_of(".") + 1);
    return result == sFilepath ? "" : result;
}

void SceneImporter::ImportMaterialTextures(Scene * scene, aiMaterial * mMaterial, Material &material)
{
    for (aiTextureType texType = aiTextureType_NONE; texType < aiTextureType_UNKNOWN; texType = aiTextureType(texType + 1))
    {
        int textureTypeCount = mMaterial->GetTextureCount(static_cast<aiTextureType>(texType));
        
        if (textureTypeCount <= 0)                                                          // only loading one
            continue;

        aiString texPath;

        if (mMaterial->GetTexture(static_cast<aiTextureType>(texType), 0, &texPath) == AI_SUCCESS)
        {
            auto filepath = scene->directory + "/" + std::string(texPath.data);             // find if texture was already loaded previously
            bool alreadyLoaded = false;
            int savedTextureIndex = 0;

            
            if (GetFileExtension(scene->filepath) == "obj" && texType == aiTextureType_HEIGHT)
                texType = aiTextureType_NORMALS;                                            // for wavefront obj we assimp bump = normal map

            for (unsigned int i = 0; i < scene->textures.size() && !alreadyLoaded; ++i)
            {
                alreadyLoaded |= scene->textures[i]->GetFilepath() == filepath;
                savedTextureIndex = i;
            }

            if (!alreadyLoaded)
            {
                auto newTexture = std::make_shared<Texture2D>();
                if (TextureImporter::ImportTexture2D(filepath, *newTexture))
                {
                    scene->textures.push_back(newTexture);
                    material.AddTexture(newTexture, static_cast<RawTexture::TextureType>(texType));
                    newTexture->textureTypes.insert(static_cast<RawTexture::TextureType>(texType));
                }
                else
                    throw std::runtime_error("Error loading texture " + filepath);
            }                                                                               // raw data from this texture has been previously loaded
            else
            {                                                                               // just add reference and associate a new texture type
                material.AddTexture(scene->textures[savedTextureIndex], static_cast<RawTexture::TextureType>(texType));
                scene->textures[savedTextureIndex]->textureTypes.insert(static_cast<RawTexture::TextureType>(texType));
            }
        }
    }
}

const std::array<const std::string, 26> SceneImporter::FlagNames =
{
    "CalculateTangentSpace",
    "JoinIdenticalVertices",
    "MakeLeftHanded",
    "Triangulate",
    "RemoveComponent",
    "GenerateNormals",
    "GenerateSmoothNormals",
    "SplitLargeMeshes",
    "PreTransformVertices",
    "LimitBoneWeights",
    "ValidateDataStructure",
    "ImproveCacheLocality",
    "RemoveRedundantMaterials",
    "FixInfacingNormals",
    "SortByType",
    "FindDegenerates",
    "FindInvalidData",
    "GenerateUVCoords",
    "TransformUVCoords",
    "FindInstances",
    "OptimizeMeshes",
    "OptimizeGraph",
    "FlipUVs",
    "FlipWindingOrder",
    "SplitByBoneCount",
    "Debone"
};
