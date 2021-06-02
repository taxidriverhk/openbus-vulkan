#pragma once

#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

class btCollisionShape;
class btMotionState;
class btRigidBody;
class btTriangleMesh;
class btVector3;

struct CollisionMesh
{
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
};

struct CollisionBody
{
    std::vector<float> vertices;
    std::vector<int> indices;
    std::unique_ptr<btTriangleMesh> mesh;
    std::unique_ptr<btCollisionShape> shape;
    std::unique_ptr<btMotionState> motionState;
    std::unique_ptr<btRigidBody> body;
};
