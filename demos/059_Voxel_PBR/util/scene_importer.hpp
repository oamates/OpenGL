#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <string>
#include <array>

struct aiLight;
struct aiCamera;
struct aiNode;
struct aiMesh;
struct aiMaterial;

struct Light;
struct Camera;
struct Node;
struct Mesh;
struct Material;
struct Scene;

// Utilizes Assimp library to import scenes info into the Scene class.
struct SceneImporter
{
    enum ImportFlags
    {
        CalculateTangentSpace = aiProcess_CalcTangentSpace,
        JoinIdenticalVertices = aiProcess_JoinIdenticalVertices,
        MakeLeftHanded = aiProcess_MakeLeftHanded,
        Triangulate = aiProcess_Triangulate,
        RemoveComponent = aiProcess_RemoveComponent,
        GenerateNormals = aiProcess_GenNormals,
        GenerateSmoothNormals = aiProcess_GenSmoothNormals,
        SplitLargeMeshes = aiProcess_SplitLargeMeshes,
        PreTransformVertices = aiProcess_PreTransformVertices,
        LimitBoneWeights = aiProcess_LimitBoneWeights,
        ValidateDataStructure = aiProcess_ValidateDataStructure,
        ImproveCacheLocality = aiProcess_ImproveCacheLocality,
        RemoveRedundantMaterials = aiProcess_RemoveRedundantMaterials,
        FixInfacingNormals = aiProcess_FixInfacingNormals,
        SortByType = aiProcess_SortByPType,
        FindDegenerates = aiProcess_FindDegenerates,
        FindInvalidData = aiProcess_FindInvalidData,
        GenerateUVCoords = aiProcess_GenUVCoords,
        TransformUVCoords = aiProcess_TransformUVCoords,
        FindInstances = aiProcess_FindInstances,
        OptimizeMeshes = aiProcess_OptimizeMeshes,
        OptimizeGraph = aiProcess_OptimizeGraph,
        FlipUVs = aiProcess_FlipUVs ,
        FlipWindingOrder = aiProcess_FlipWindingOrder,
        SplitByBoneCount = aiProcess_SplitByBoneCount,
        Debone = aiProcess_Debone,
    };

    static const std::array<const std::string, 26> FlagNames;	
    static bool Import(const std::string &filepath, Scene * scene, unsigned flags);     // Imports a scene from the specified filepath.

    SceneImporter() {}
    virtual ~SceneImporter() {}

	static void ImportMaterial(aiMaterial * mMaterial, Material &material);
    static void ImportMesh(aiMesh * mMesh, Mesh &mesh);
    static void ProcessNodes(Scene * scene, aiNode * mNode, Node &node);
    static void ImportCamera(aiCamera * mCam, Camera &camera);
    static void ImportLight(aiLight * mLight, Light &light);
    static void ImportMaterialTextures(Scene * scene, aiMaterial * mMaterial, Material &material);
};