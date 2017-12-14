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
        enum GeometryState
        {
            Dynamic,
            Static,
        };
        // The node geometry state, flags this mesh as dynamic or static
        GeometryState nodeState;
        /// Updates the transform matrix, the boundaries
        /// and the inverse transpose
        void UpdateTransformMatrix() override;
        /// Node world space boundaries
        BoundingBox boundaries;
        /// All the node meshes
        std::vector<std::shared_ptr<MeshDrawer>> meshes;
        /// All the subnodes to this node
        std::vector<std::shared_ptr<Node>> nodes;
        /// If <see cref="BuildDrawList"/>  has been called before,
        /// at least once. Draws all the meshes associated with this
        ///  node and its subnodes.
        void DrawList() const;
        /// Same as <see cref="DrawList"/> but only draws the
        /// nodes that meet the given <see cref="GeometryState"/>
        void DrawListState(GeometryState state) const;
        /// Builds a draw list from all the meshes associated with
        /// this node and its subnodes
        void BuildDrawList();
        /// Node's inverse transpose matrix
        const glm::mat4 &InverseTranspose() const;
        /// The node's draw list
        const std::vector<Node *> &DrawListNodes() const;

        Node();
        virtual ~Node();

        glm::mat4 inverseTransposeModel;
        std::vector<Node *> drawList;
        /// Builds the draw list.
        void BuildDrawList(std::vector<Node *> &base);
        /// Draws the node's meshes.
        void DrawMeshes() const;
        /// Updates the node's boundaries and the boundaries
        /// of the node's meshes accordly with the
        void TransformBoundaries();
};

