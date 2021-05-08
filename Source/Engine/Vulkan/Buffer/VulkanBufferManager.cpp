#include <assert.h>

#define VMA_IMPLEMENTATION

#include "Engine/Image.h"
#include "Engine/Material.h"
#include "Engine/Mesh.h"
#include "Engine/Terrain.h"
#include "Engine/Vulkan/VulkanContext.h"
#include "Engine/Vulkan/VulkanRenderPass.h"
#include "Engine/Vulkan/Command/VulkanCommand.h"
#include "Engine/Vulkan/Image/VulkanImage.h"
#include "Engine/Vulkan/Pipeline/VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanBufferManager.h"

VulkanBufferManager::VulkanBufferManager(
    VulkanContext *context,
    VulkanRenderPass *renderPass,
    VulkanDrawingPipelines pipelines,
    VkCommandPool commandPool,
    uint32_t frameBufferSize)
    : context(context),
      renderPass(renderPass),
      pipelines(pipelines),
      frameBufferSize(frameBufferSize),
      commandPool(commandPool),
      uniformBufferUpdated(true),
      entityBufferCache{},
      terrainBufferCache{},
      cubeMapBufferCache{}
{
}

VulkanBufferManager::~VulkanBufferManager()
{
}

void VulkanBufferManager::Create()
{
    CreateDescriptorPool();
    CreateMemoryAllocator();
    CreateUniformBuffers();
}

void VulkanBufferManager::Destroy()
{
    DestroyUniformBuffers();
    DestroyCubeMapBuffer();

    // Destroying the descriptor pool will automatically free the descriptor sets
    // created using the pool, so no need to explicitly free each descriptor set
    vkDestroyDescriptorPool(context->GetLogicalDevice(), descriptorPool, nullptr);

    vmaDestroyAllocator(vmaAllocator);
    vmaDestroyAllocator(imageVmaAllocator);
}

VulkanDrawingBuffer VulkanBufferManager::GetDrawingBuffer(uint32_t imageIndex)
{
    VulkanDrawingBuffer drawingBuffer{};
    drawingBuffer.cubeMapBuffer = cubeMapBufferCache;
    drawingBuffer.uniformBuffer = uniformBuffers[imageIndex].get();
    for (auto &entry : entityBufferCache)
    {
        drawingBuffer.entityBuffers.push_back(entry.second);
    }
    for (auto &entry : terrainBufferCache)
    {
        drawingBuffer.terrainBuffers.push_back(entry.second);
    }
    for (auto &entry : screenObjectBufferCache)
    {
        drawingBuffer.screenObjectBuffers.push_back(entry.second);
    }
    return drawingBuffer;
}

void VulkanBufferManager::LoadCubeMapBuffer(
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    std::vector<Image *> &images)
{
    assert(images.size() > 0);

    cubeMapVertexBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
    cubeMapVertexBuffer->Load(
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertices.data(),
        static_cast<uint32_t>(sizeof(Vertex) * vertices.size()));

    cubeMapIndexBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
    cubeMapIndexBuffer->Load(
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indices.data(),
        static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));

    std::vector<uint8_t *> imagePixels;
    for (Image *image : images)
    {
        imagePixels.push_back(image->GetPixels());
    }
    cubeMapImage = std::make_unique<VulkanImage>(
        context, commandPool, imageVmaAllocator, VulkanImageType::CubeMap);
    cubeMapImage->Load(
        imagePixels,
        images[0]->GetWidth(),
        images[0]->GetHeight(),
        descriptorPool,
        pipelines.cubeMapPipeline->GetImageDescriptorSetLayout());

    cubeMapBufferCache.imageBuffer = cubeMapImage.get();
    cubeMapBufferCache.vertexBuffer = cubeMapVertexBuffer.get();
    cubeMapBufferCache.indexBuffer = cubeMapIndexBuffer.get();
}

void VulkanBufferManager::LoadIntoBuffer(
    uint32_t instanceId,
    uint32_t meshId,
    uint32_t imageId,
    VulkanInstanceBufferInput &instanceBufferInput,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    Material *material)
{
    if (vertexBuffers.count(meshId) == 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        vertexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertices.data(),
            static_cast<uint32_t>(sizeof(Vertex) * vertices.size()));
        vertexBuffers.insert(std::make_pair(meshId, std::move(vertexBuffer)));

        std::shared_ptr<VulkanBuffer> indexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        indexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indices.data(),
            static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
        indexBuffers.insert(std::make_pair(meshId, std::move(indexBuffer)));

        vertexBufferCount[meshId] = 1;
    }
    else
    {
        vertexBufferCount[meshId] = vertexBufferCount[meshId] + 1;
    }

    if (imageBuffers.count(imageId) == 0)
    {
        // TODO: only load the diffuse image for now
        std::shared_ptr<VulkanImage> diffuseImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        std::shared_ptr<Image> imageToLoad = material->diffuseImage;
        diffuseImage->Load(
            std::vector<uint8_t *>({ imageToLoad->GetPixels() }),
            imageToLoad->GetWidth(),
            imageToLoad->GetHeight(),
            descriptorPool,
            pipelines.staticPipeline->GetImageDescriptorSetLayout());
        imageBuffers.insert(std::make_pair(imageId, std::move(diffuseImage)));

        imageBufferCount[imageId] = 1;
    }
    else
    {
        imageBufferCount[imageId] = imageBufferCount[imageId] + 1;
    }

    std::shared_ptr<VulkanBuffer> instanceBuffer = std::make_shared<VulkanBuffer>(
        context, commandPool, vmaAllocator);
    instanceBuffer->Load(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &instanceBufferInput,
        sizeof(VulkanInstanceBufferInput));
    instanceBuffer->CreateDescriptorSet(
        descriptorPool,
        pipelines.staticPipeline->GetInstanceDescriptorSetLayout(),
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        sizeof(VulkanInstanceBufferInput));
    instanceBuffers.insert(std::make_pair(instanceId, std::move(instanceBuffer)));

    VulkanEntityBufferIds bufferIds{};
    bufferIds.instanceBufferId = instanceId;
    bufferIds.vertexBufferId = meshId;
    bufferIds.indexBufferId = meshId;
    bufferIds.imageBufferId = imageId;
    bufferIdCache.insert(std::make_pair(instanceId, bufferIds));

    VulkanEntityBuffer entityBuffer{};
    entityBuffer.instanceBuffer = instanceBuffers[bufferIds.instanceBufferId].get();
    entityBuffer.vertexBuffer = vertexBuffers[bufferIds.vertexBufferId].get();
    entityBuffer.indexBuffer = indexBuffers[bufferIds.indexBufferId].get();
    entityBuffer.imageBuffer = imageBuffers[bufferIds.imageBufferId].get();
    entityBufferCache.insert(std::make_pair(instanceId, entityBuffer));
}

void VulkanBufferManager::LoadScreenObjectBuffer(
    uint32_t screenObjectId,
    std::vector<ScreenObjectVertex> &vertices,
    Image *image)
{
    if (screenObjectBuffers.count(screenObjectId) == 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        vertexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertices.data(),
            static_cast<uint32_t>(sizeof(ScreenObjectVertex) * vertices.size()));
        screenObjectBuffers.insert(std::make_pair(screenObjectId, std::move(vertexBuffer)));

        std::shared_ptr<VulkanImage> screenImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        screenImage->Load(
            std::vector<uint8_t *>({ image->GetPixels() }),
            image->GetWidth(),
            image->GetHeight(),
            descriptorPool,
            pipelines.screenPipeline->GetImageDescriptorSetLayout());
        imageBuffers.insert(std::make_pair(screenObjectId, std::move(screenImage)));
    }
    else
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = screenObjectBuffers[screenObjectId];
        vertexBuffer->Update(
            vertices.data(),
            static_cast<uint32_t>(sizeof(ScreenObjectVertex) * vertices.size()));
    }

    VulkanScreenObjectBuffer screenObjectBuffer{};
    screenObjectBuffer.vertexBuffer = screenObjectBuffers[screenObjectId].get();
    screenObjectBuffer.imageBuffer = imageBuffers[screenObjectId].get();
    screenObjectBufferCache.insert(std::make_pair(screenObjectId, screenObjectBuffer));
}

void VulkanBufferManager::LoadTerrainIntoBuffer(
    uint32_t terrainId,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    Image *texture)
{
    if (terrainVertexBuffers.count(terrainId) == 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        vertexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertices.data(),
            static_cast<uint32_t>(sizeof(Vertex) * vertices.size()));
        terrainVertexBuffers.insert(std::make_pair(terrainId, std::move(vertexBuffer)));

        std::shared_ptr<VulkanBuffer> indexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        indexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indices.data(),
            static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
        terrainIndexBuffers.insert(std::make_pair(terrainId, std::move(indexBuffer)));

        std::shared_ptr<VulkanImage> terrainImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        terrainImage->Load(
            std::vector<uint8_t *>({ texture->GetPixels() }),
            texture->GetWidth(),
            texture->GetHeight(),
            descriptorPool,
            pipelines.staticPipeline->GetImageDescriptorSetLayout());
        imageBuffers.insert(std::make_pair(terrainId, std::move(terrainImage)));
        imageBufferCount[terrainId] = 1;
    }
    else
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = terrainVertexBuffers[terrainId];
        std::shared_ptr<VulkanBuffer> indexBuffer = terrainIndexBuffers[terrainId];

        vertexBuffer->Update(vertices.data(), static_cast<uint32_t>(sizeof(Vertex) * vertices.size()));
        indexBuffer->Update(indices.data(), static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
    }

    VulkanTerrainBuffer terrainBuffer{};
    terrainBuffer.vertexBuffer = terrainVertexBuffers[terrainId].get();
    terrainBuffer.indexBuffer = terrainIndexBuffers[terrainId].get();
    terrainBuffer.imageBuffer = imageBuffers[terrainId].get();
    terrainBufferCache.insert(std::make_pair(terrainId, terrainBuffer));
}

void VulkanBufferManager::ResetCommandPool(VkCommandPool commandPool)
{
    this->commandPool = commandPool;
}

void VulkanBufferManager::ResetScreenBuffers()
{
    int width = context->GetScreen()->GetWidth(),
        height = context->GetScreen()->GetHeight();
    glm::mat4 projection = glm::ortho(0, width, 0, height);
    screenBufferInput.projection = projection;
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        screenBuffers[i]->Update(&screenBufferInput, sizeof(VulkanScreenBufferInput));
    }
}

void VulkanBufferManager::UnloadBuffer(uint32_t instanceId)
{
    if (bufferIdCache.count(instanceId) > 0)
    {
        VulkanEntityBufferIds bufferIds = bufferIdCache[instanceId];
        
        uint32_t instanceBufferId = bufferIds.instanceBufferId;
        std::shared_ptr<VulkanBuffer> instanceBuffer = instanceBuffers[instanceBufferId];
        instanceBuffer->Unload();
        instanceBuffers.erase(instanceBufferId);

        uint32_t vertexBufferId = bufferIds.vertexBufferId;
        vertexBufferCount[vertexBufferId] = vertexBufferCount[vertexBufferId] - 1;
        if (vertexBufferCount[vertexBufferId] == 0)
        {
            std::shared_ptr<VulkanBuffer> vertexBuffer = vertexBuffers[vertexBufferId];
            vertexBuffer->Unload();
            vertexBuffers.erase(vertexBufferId);

            std::shared_ptr<VulkanBuffer> indexBuffer = indexBuffers[vertexBufferId];
            indexBuffer->Unload();
            indexBuffers.erase(vertexBufferId);
        }

        uint32_t imageBufferId = bufferIds.imageBufferId;
        imageBufferCount[imageBufferId] = imageBufferCount[imageBufferId] - 1;
        if (imageBufferCount[imageBufferId] == 0)
        {
            std::shared_ptr<VulkanImage> imageBuffer = imageBuffers[imageBufferId];
            imageBuffer->Unload();
            imageBuffers.erase(imageBufferId);
        }

        bufferIdCache.erase(instanceId);
        entityBufferCache.erase(instanceId);
    }
}

void VulkanBufferManager::UnloadScreenObjectBuffer(uint32_t screenObjectId)
{
    if (screenObjectBuffers.count(screenObjectId) > 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = screenObjectBuffers[screenObjectId];
        vertexBuffer->Unload();
        screenObjectBuffers.erase(screenObjectId);

        std::shared_ptr<VulkanImage> imageBuffer = imageBuffers[screenObjectId];
        imageBuffer->Unload();
        imageBuffers.erase(screenObjectId);

        screenObjectBufferCache.erase(screenObjectId);
    }
}

void VulkanBufferManager::UnloadTerrainBuffer(uint32_t terrainId)
{
    if (terrainVertexBuffers.count(terrainId) > 0)
    {
        std::shared_ptr<VulkanBuffer> vertexBuffer = terrainVertexBuffers[terrainId];
        vertexBuffer->Unload();
        terrainVertexBuffers.erase(terrainId);

        std::shared_ptr<VulkanBuffer> indexBuffer = terrainIndexBuffers[terrainId];
        indexBuffer->Unload();
        terrainIndexBuffers.erase(terrainId);

        std::shared_ptr<VulkanImage> imageBuffer = imageBuffers[terrainId];
        imageBuffer->Unload();
        imageBuffers.erase(terrainId);

        terrainBufferCache.erase(terrainId);
    }
}

void VulkanBufferManager::UpdateInstanceBuffer(uint32_t instanceId, VulkanInstanceBufferInput input)
{
    if (instanceBuffers.count(instanceId) == 0)
    {
        return;
    }
    instanceBuffers[instanceId]->Update(&input, sizeof(VulkanInstanceBufferInput));
}

void VulkanBufferManager::UpdateUniformBuffer(VulkanUniformBufferInput input)
{
    uniformBufferInput = input;
    
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        uniformBuffers[i]->Update(&uniformBufferInput, sizeof(VulkanUniformBufferInput));
    }

    uniformBufferUpdated = true;
}

void VulkanBufferManager::CreateDescriptorPool()
{
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = STATIC_PIPELINE_DESCRIPTOR_POOL_SIZE_COUNT;
    poolInfo.pPoolSizes = STATIC_PIPELINE_DESCRIPTOR_POOL_SIZES;
    poolInfo.maxSets = MAX_DESCRIPTOR_SETS;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateDescriptorPool(context->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool),
        "Failed to create descriptor pool");
}

void VulkanBufferManager::CreateMemoryAllocator()
{
    VmaAllocatorCreateInfo createInfo{};
    createInfo.device = context->GetLogicalDevice();
    createInfo.physicalDevice = context->GetPhysicalDevice();
    createInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    createInfo.instance = context->GetInstance();

    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateAllocator(&createInfo, &vmaAllocator),
        "Failed to create VMA allocator");
    ASSERT_VK_RESULT_SUCCESS(
        vmaCreateAllocator(&createInfo, &imageVmaAllocator),
        "Failed to create image VMA allocator");
}

void VulkanBufferManager::CreateScreenBuffers()
{
    int width = context->GetScreen()->GetWidth(),
        height = context->GetScreen()->GetHeight();
    glm::mat4 projection = glm::ortho(0, width, 0, height);
    screenBufferInput.projection = projection;

    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        std::unique_ptr<VulkanBuffer> screenBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
        screenBuffer->Load(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &screenBufferInput,
            sizeof(VulkanScreenBufferInput));
        screenBuffer->CreateDescriptorSet(
            descriptorPool,
            pipelines.screenPipeline->GetUniformDescriptorSetLayout(),
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            sizeof(VulkanScreenBufferInput));
        screenBuffers.push_back(std::move(screenBuffer));
    }
}

void VulkanBufferManager::CreateUniformBuffers()
{
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        std::unique_ptr<VulkanBuffer> uniformBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
        VulkanUniformBufferInput defaultUniformBufferInput{};
        uniformBuffer->Load(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &defaultUniformBufferInput,
            sizeof(VulkanUniformBufferInput));
        uniformBuffer->CreateDescriptorSet(
            descriptorPool,
            pipelines.staticPipeline->GetUniformDescriptorSetLayout(),
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            sizeof(VulkanUniformBufferInput));
        uniformBuffers.push_back(std::move(uniformBuffer));
    }
}

void VulkanBufferManager::DestroyCubeMapBuffer()
{
    cubeMapImage->Unload();
    cubeMapIndexBuffer->Unload();
    cubeMapVertexBuffer->Unload();
}

void VulkanBufferManager::DestroyUniformBuffers()
{
    for (std::unique_ptr<VulkanBuffer> &uniformBuffer : uniformBuffers)
    {
        uniformBuffer->Unload();
    }
    uniformBuffers.clear();
}
