#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "vk_mem_alloc.hpp"

#include "Engine/DrawEngine.h"
#include "Engine/Screen.h"
#include "Engine/Vertex.h"
#include "VulkanCommon.h"
#include "VulkanContext.h"

class VulkanFrameBuffer;
class VulkanPipeline;
class VulkanBufferManager;
class VulkanCommandPool;
class VulkanCommandManager;
class VulkanPipelineManager;
class VulkanRenderPass;

class VulkanDrawEngine : public DrawEngine
{
public:
    VulkanDrawEngine(Screen *screen, bool enableDebugging);
    ~VulkanDrawEngine();

    void Destroy() override;
    void DrawFrame() override;
    void Initialize() override;
    void LoadCubeMap(CubeMap &cubeMap) override;
    void LoadEntity(const Entity &entity) override;
    void LoadLineSegments(std::vector<LineSegmentVertex> &lines) override;
    void LoadScreenObject(ScreenMesh &screenMesh) override;
    void LoadTerrain(Terrain &terrain) override;
    void SetFog(float density, float gradient) override;
    void UpdateCamera(Camera *camera) override;
    void UpdateEntityTransformation(uint32_t entityId, EntityTransformation transformation) override;
    void UnloadEntity(uint32_t entityId) override;
    void UnloadScreenObject(uint32_t screenMeshId) override;
    void UnloadTerrain(uint32_t terrainId) override;

private:
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // The game treats Z-axis as the upward axis, while Vulkan/OpenGL treats Y-axis as the upward axis
    // Therefore, a conversion is required.
    // Texture v-coordinate must also be flipped as the y-axis texture viewport in Vulkan is flipped.
    inline static Vertex ConvertToVulkanVertex(const Vertex &input)
    {
        return
        {
            { input.position.x, input.position.z, input.position.y },
            { input.normal.x, input.normal.z, input.normal.y },
            { input.uv.x, 1.0f - input.uv.y }
        };
    }

    inline static glm::vec3 ConvertToVulkanCoordinates(const glm::vec3 &input)
    {
        return { input.x, input.z, -input.y };
    }

    // Only used by the wheels of the raycast vehicles in Bullet Physics
    // where the rotation is only correct from the matrix generated for OpenGL
    inline static glm::mat4 ConvertToVulkanTransformationMatrix(const glm::mat4 &input)
    {
        glm::mat4 originalWithoutTranslation = input;
        glm::vec3 translation = ConvertToVulkanCoordinates({ input[3][0], input[3][1], input[3][2] });
        // Make the orientation change happen about the origin (i.e. before translation)
        // to swap the rotation about y-axis and that about z-axis
        // (rotate the object 90 degrees about x-axis, then rotate 180 degrees about y-axis, then rotate 180 degrees about z-axis)
        originalWithoutTranslation[3][0] = originalWithoutTranslation[3][1] = originalWithoutTranslation[3][2] = 0;
        glm::mat4 orientationChange
        {
            { 1, 0, 0, 0 },
            { 0, 0, -1, 0 },
            { 0, 1, 0, 0 },
            { translation.x, translation.y, translation.z, 1 }
        };
        return orientationChange * originalWithoutTranslation;
    }

    inline static glm::mat4 ComputeTransformationMatrix(
        const glm::vec3 &translation,
        const glm::vec3 &scale,
        const glm::vec3 &rotationAngles
    )
    {
        // Transformation order (from right to left, extrinsic rotations)
        // scale * translation * rotateZ * rotateY * rotateX
        const float c1 = glm::cos(glm::radians<float>(rotationAngles.x));
        const float c2 = glm::cos(glm::radians<float>(-rotationAngles.y));
        const float c3 = glm::cos(glm::radians<float>(-rotationAngles.z));
        const float s1 = glm::sin(glm::radians<float>(rotationAngles.x));
        const float s2 = glm::sin(glm::radians<float>(-rotationAngles.y));
        const float s3 = glm::sin(glm::radians<float>(-rotationAngles.z));
        return glm::mat4
        {
            {
                scale.x * (c1 * c2),
                scale.x * (c2 * s1),
                scale.x * -(s2),
                0.0f
            },
            {
                scale.y * (c1 * s2 * s3 - c3 * s1),
                scale.y * (c1 * c3 + s1 * s2 * s3),
                scale.y * (c2 * s3),
                0.0f
            },
            {
                scale.z * (s1 * s3 + c1 * c3 * s2),
                scale.z * (c3 * s1 * s2 - c1 * s3),
                scale.z * (c2 * c3),
                0.0f
            },
            {
                translation.x,
                translation.y,
                translation.z,
                1.0f
            }
        };
    }

    inline static glm::mat4 ComputeTransformationMatrix(
        const glm::vec3 &translation,
        const glm::vec3 &scale,
        const glm::vec3 &rotationAxis,
        float rotationAngle
    )
    {
        glm::mat4 identitiyMatrix = glm::identity<glm::mat4>();
        glm::mat4 transformationMatrix = glm::translate(identitiyMatrix, translation);
        glm::mat4 rotationMatrix = glm::rotate(identitiyMatrix, glm::radians<float>(rotationAngle), rotationAxis);
        transformationMatrix = transformationMatrix * rotationMatrix;
        return glm::scale(transformationMatrix, scale);
    }

    void CreateCommandBuffers();
    void CreateFrameBuffers();
    void CreateSynchronizationObjects();
    void ClearDrawingBuffers();
    void DestroyCommandBuffers();
    void DestroyFrameBuffers();
    void DestroySynchronizationObjects();

    void BeginFrame(uint32_t &imageIndex);
    void EndFrame(uint32_t &imageIndex);
    void Submit(uint32_t &imageIndex);

    void RecreateSwapChain();

    void MarkDataAsUpdated();

    bool isInitialized;
    bool enableDebugging;

    Screen *screen;
    std::unique_ptr<VulkanContext> context;
    std::unique_ptr<VulkanRenderPass> renderPass;

    std::unique_ptr<VulkanCommandPool> commandPool;
    std::unique_ptr<VulkanCommandManager> commandManager;
    std::unique_ptr<VulkanPipelineManager> pipelineManager;

    std::unordered_set<uint32_t> bufferIds;
    std::unordered_set<uint32_t> terrainBufferIds;
    std::unordered_set<uint32_t> screenObjectIds;
    std::unique_ptr<VulkanBufferManager> bufferManager;

    // Push constants
    VulkanPushConstants pushConstants;

    // Used for frame buffer
    VmaAllocator frameBufferImageAllocator;

    // Based on number of swap chain images (which is usually 3)
    std::vector<std::unique_ptr<VulkanFrameBuffer>> screenFrameBuffers;
    std::vector<VkFence> imagesInFlight;
    std::vector<bool> dataUpdated;

    // Based on the maximum allowed frames in flight
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    // Uniform/instance buffers that are to be submitted in the next draw call
    VulkanUniformBufferInput uniformBufferInput;
    std::unordered_map<uint32_t, VulkanInstanceBufferInput> instanceBufferInputs;

    // Active frame that is in use by the GPU
    uint32_t currentInFlightFrame;
};
