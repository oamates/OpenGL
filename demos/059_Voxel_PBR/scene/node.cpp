#include "node.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "mesh.hpp"
#include "camera.hpp"
#include "../core/renderer.hpp"

#include <glm/detail/func_matrix.hpp>

void Node::UpdateTransformMatrix()
{
    Transform::UpdateTransformMatrix();
    // per transform change update inversetranpose
    inverseTransposeModel = inverse(transpose(Matrix()));
    // update boundaries with transform
    TransformBoundaries();
}

Node::Node()
{
    name = "Default Node";
    inverseTransposeModel = inverse(transpose(Matrix()));
    nodeState = Static;
}

Node::~Node()
{
}

void Node::BuildDrawList(std::vector<Node *> &base)
{
    base.push_back(this);

    for (auto &node : nodes)
    {
        node->BuildDrawList(base);
    }
}

void Node::DrawMeshes() const
{
    static const auto &renderer = Renderer::Active();

    for (const auto &mesh : meshes)
    {
        if (!mesh->IsLoaded()) { return; }

        renderer->SetMaterialUniforms(*mesh->material);
        mesh->DrawElements();
    }
}

void Node::DrawList() const
{
    static const auto &renderer = Renderer::Active();;
    static const auto &camera = Camera::Active();

    // draw elements using draw list
    for (auto &node : drawList)
    {
        if (!camera->InFrustum(node->boundaries))
        {
            continue;
        }

        // set matrices uniform with updated matrices
        renderer->SetMatricesUniforms(*node);
        // draw node meshes
        node->DrawMeshes();
    }
}

void Node::DrawListState(GeometryState state) const
{
    static const auto &renderer = Renderer::Active();;
    static const auto &camera = Camera::Active();

    // draw elements using draw list
    for (auto &node : drawList)
    {
        if (node->nodeState != state) { continue; }

        if (!camera->InFrustum(node->boundaries))
        {
            continue;
        }

        // set matrices uniform with updated matrices
        renderer->SetMatricesUniforms(*node);
        // draw node meshes
        node->DrawMeshes();
    }
}

void Node::BuildDrawList()
{
    drawList.clear();
    BuildDrawList(drawList);
}

const glm::mat4 &Node::InverseTranspose() const
{
    return inverseTransposeModel;
}

const std::vector<Node *> &Node::DrawListNodes() const
{
    return drawList;
}

void Node::TransformBoundaries()
{
    boundaries.Transform(transform.Matrix());

    for (auto &mesh : meshes)
    {
        mesh->boundaries.Transform(transform.Matrix());
    }
}
