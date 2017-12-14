#pragma once

#include "../types/scene_object.hpp"
#include "../types/bounding_box.hpp"
#include "../util/single_active.hpp"

#include <vector>

struct Camera;
struct MeshDrawer;

// Describes a node in scene. A node can contain an undefined number of meshes and subnodes.
struct Node : public SceneObject
{
    enum GeometryState { Dynamic, Static };

    glm::mat4 inverseTransposeModel;
    std::vector<Node *> drawList;
    GeometryState nodeState;                            // The node geometry state, flags this mesh as dynamic or static

    std::vector<std::shared_ptr<MeshDrawer>> meshes;    // All the node meshes
    std::vector<std::shared_ptr<Node>> nodes;           // All the subnodes to this node
        
    BoundingBox boundaries;                             // Node world space boundaries

    Node();
    virtual ~Node();
        
    void UpdateTransformMatrix() override;              // Updates the transform matrix, the boundaries and the inverse transpose
    void DrawList() const;                              // If BuildDrawList has been called before, at least once. Draws all the meshes associated with this node and its subnodes.
    void DrawListState(GeometryState state) const;      // Same as DrawList but only draws the nodes that meet the given GeometryState
    void BuildDrawList();                               // Builds a draw list from all the meshes associated with this node and its subnodes
    const glm::mat4 &InverseTranspose() const;          // Node's inverse transpose matrix
    const std::vector<Node *> &DrawListNodes() const;   // The node's draw list
    void BuildDrawList(std::vector<Node *> &base);      // Builds the draw list.
    void DrawMeshes() const;                            // Draws the node's meshes.
    void TransformBoundaries();                         // Updates the node's boundaries and the boundaries of the node's meshes accordly with the
};