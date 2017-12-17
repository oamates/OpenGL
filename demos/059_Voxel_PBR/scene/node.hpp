#pragma once

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/detail/func_matrix.hpp>

#include "../core/renderer.hpp"
#include "../types/scene_object.hpp"
#include "../types/bbox.hpp"
#include "../util/single_active.hpp"

#include "mesh.hpp"
#include "camera.hpp"

// Describes a node in scene. A node can contain an undefined number of meshes and subnodes.
struct Node : public SceneObject
{
    enum GeometryState { Dynamic, Static };

    glm::mat4 inverseTransposeModel;
    std::vector<Node *> drawList;
    GeometryState nodeState;                                                // The node geometry state, flags this mesh as dynamic or static

    std::vector<std::shared_ptr<MeshDrawer>> meshes;                        // All the node meshes
    std::vector<std::shared_ptr<Node>> nodes;                               // All the subnodes to this node
        
    bbox_t boundaries;                                                      // Node world space boundaries

    Node()
    {
        name = "Default Node";
        inverseTransposeModel = glm::inverse(glm::transpose(Matrix()));
        nodeState = Static;
    }

    virtual ~Node() {}
        
    void UpdateTransformMatrix() override                                   // Updates the transform matrix, the boundaries and the inverse transpose
    {
        Transform::UpdateTransformMatrix();
        inverseTransposeModel = glm::inverse(glm::transpose(Matrix()));     // per transform change update inversetranpose
        TransformBoundaries();                                              // update boundaries with transform
    }

    void DrawList() const                                                   // If BuildDrawList has been called before, at least once. Draws all the meshes associated with this node and its subnodes.
    {
        static const auto &renderer = Renderer::Active();;
        static const auto &camera = Camera::Active();
        for (auto &node : drawList)                                         // draw elements using draw list
        {
            if (!camera->InFrustum(node->boundaries))
                continue;
            renderer->SetMatricesUniforms(*node);                           // set matrices uniform with updated matrices
            node->DrawMeshes();                                             // draw node meshes
        }
    }

    void DrawListState(GeometryState state) const                           // Same as DrawList but only draws the nodes that meet the given GeometryState
    {
        static const auto &renderer = Renderer::Active();;
        static const auto &camera = Camera::Active();
        for (auto &node : drawList)                                         // draw elements using draw list
        {
            if ((node->nodeState != state) || (!camera->InFrustum(node->boundaries)))
                continue;
            renderer->SetMatricesUniforms(*node);                           // set matrices uniform with updated matrices
            node->DrawMeshes();                                             // draw node meshes
        }
    }


    void BuildDrawList()                                                    // Builds a draw list from all the meshes associated with this node and its subnodes
    {
        drawList.clear();
        BuildDrawList(drawList);
    }

    const glm::mat4& InverseTranspose() const                               // Node's inverse transpose matrix
        { return inverseTransposeModel; }
 
    const std::vector<Node*>& DrawListNodes() const                         // The node's draw list
        { return drawList; }

    void BuildDrawList(std::vector<Node *> &base)                           // Builds the draw list.
    {
        base.push_back(this);
        for (auto &node : nodes)
            node->BuildDrawList(base);
    }

    void DrawMeshes() const                                                 // Draws the node's meshes.
    {
        static const auto &renderer = Renderer::Active();
        for (const auto &mesh : meshes)
        {
            if (!mesh->IsLoaded())
                return;
            renderer->SetMaterialUniforms(*mesh->material);
            mesh->DrawElements();
        }
    }

    void TransformBoundaries()                                              // Updates the node's boundaries and the boundaries of the node's meshes
    {
        boundaries.Transform(transform.Matrix());
        for (auto &mesh : meshes)
            mesh->boundaries.Transform(transform.Matrix());
    }
};