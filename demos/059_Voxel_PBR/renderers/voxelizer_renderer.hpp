#pragma once
#include "../core/renderer.hpp"

#include <oglplus/texture.hpp>
#include <glm/mat4x4.hpp>
#include <oglplus/vertex_array.hpp>
#include "../programs/propagation_program.hpp"

struct ClearDynamicProgram;
struct MipmappingVolumeProgram;
struct MipmappingBaseProgram;
struct InjectRadianceProgram;
struct bbox_t;
struct VoxelDrawerProgram;
struct VoxelizationProgram;

struct VoxelizerRenderer : public Renderer
{
    static bool ShowVoxels;
    oglplus::Texture staticFlag;                                                    // output texture
    oglplus::Texture voxelAlbedo;
    oglplus::Texture voxelNormal;
    oglplus::Texture voxelEmissive;
    oglplus::Texture voxelRadiance;
    std::array<oglplus::Texture, 6> voxelTexMipmap;
    oglplus::VertexArray voxelDrawerArray;                                          // vertex buffer object for 3d texture visualization
    std::array<glm::mat4x4, 3> viewProjectionMatrix;
    std::array<glm::mat4x4, 3> viewProjectionMatrixI;
    unsigned int volumeDimension;
    unsigned int voxelCount;
    bool injectFirstBounce;
    float volumeGridSize;
    float voxelSize;
    int framestep;
    bool traceShadowCones;
    bool normalWeightedLambert;
    float traceShadowHit;
    unsigned int drawMipLevel;
    unsigned int drawDirection;
    glm::vec4 drawColorChannels;

    void Render() override;                                                         // Voxelizes the current scene. It can also be used to show the result volume.
    void SetMatricesUniforms(const Node &node) const override;                      // Sets the matrices uniforms.
    void SetMaterialUniforms(const Material &material) const override;              // Sets the material uniforms.
    void SetUpdateFrequency(const int framestep);                                   // Sets the voxelization update frequency, voxelize scene will be called every framestep number of frames.
    void VoxelizeStaticScene();
    explicit VoxelizerRenderer(RenderWindow &window);                               // Initializes a new instance of the VoxelRenderer class.
    void SetupVoxelVolumes(const unsigned int &dimension);
    void RevoxelizeScene();
    void UpdateRadiance();
    void SetupDrawVoxels(const unsigned &level, const unsigned &direction, const glm::vec4 colors);        
    ~VoxelizerRenderer();                                                           // Finalizes an instance of the VoxelRenderer class.
    const unsigned int &VolumeDimension() const;
    oglplus::Texture &VoxelRadiance();
    std::array<oglplus::Texture, 6> &VoxelTextureMipmap();
    oglplus::Texture &VoxelNormalVisibility();
    const float &VoxelWorldSize() const;
    const float &VolumeGridSize() const;
    bool TraceShadowCones() const;
    void TraceShadowCones(bool val);
    bool InjectFirstBounce() const;
    void InjectFirstBounce(bool val);
    bool WeightedLambert() const;
    void WeightedLambert(bool val);
    void TraceShadowHit(const float &umbra);
    const float &TraceShadowHit() const;
    static VoxelizationProgram& VoxelizationPass();                                 // Returns the voxelization program shader.
    static ClearDynamicProgram& VoxelCleanerShader();                               // Returns the voxelization program shader.
    static VoxelDrawerProgram& VoxelDrawerShader();
    static InjectRadianceProgram& InjectRadianceShader();
    static PropagationProgram& InjectPropagationShader();
    static MipmappingBaseProgram& MipMappingBaseShader();
    static MipmappingVolumeProgram& MipMappingVolumeShader();
    void UpdateProjectionMatrices(const bbox_t &sceneBox);                          // Creates the view projection matrices per x, y and z axis, depending on the voxel volume grid size.
    void VoxelizeDynamicScene();                                                    // Voxelizes the scene.
    void InjectRadiance();
    void GenerateMipmap();
    void GenerateMipmapVolume();
    void GenerateMipmapBase(oglplus::Texture &baseTexture);
    void ClearDynamicVoxels();
    void DrawVoxels();                                                              // Draws the resulting voxels.
};
