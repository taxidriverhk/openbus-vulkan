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
#include "Engine/Vulkan/Image/VulkanTexture.h"
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
      cubeMapBufferLoaded(false),
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
    CreateScreenBuffers();
}

void VulkanBufferManager::Destroy()
{
    DestroyScreenBuffers();
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
    drawingBuffer.screenBuffer = screenBuffers[imageIndex].get();
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
    cubeMapImage = std::make_shared<VulkanImage>(
        context, commandPool, imageVmaAllocator, VulkanImageType::CubeMap);
    cubeMapImage->Load(
        imagePixels,
        images[0]->GetWidth(),
        images[0]->GetHeight());

    cubeMapTexture = std::make_unique<VulkanTexture>(
        context,
        descriptorPool,
        pipelines.cubeMapPipeline->GetImageDescriptorSetLayout());
    cubeMapTexture->Create();
    cubeMapTexture->AddImage(cubeMapImage, 0);

    cubeMapBufferCache.textureBuffer = cubeMapTexture.get();
    cubeMapBufferCache.vertexBuffer = cubeMapVertexBuffer.get();
    cubeMapBufferCache.indexBuffer = cubeMapIndexBuffer.get();

    cubeMapBufferLoaded = true;
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
        vertexBuffers[meshId] = std::move(vertexBuffer);

        std::shared_ptr<VulkanBuffer> indexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        indexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indices.data(),
            static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
        indexBuffers[meshId] = std::move(indexBuffer);

        vertexBufferCount[meshId] = 1;
    }
    else
    {
        vertexBufferCount[meshId] = vertexBufferCount[meshId] + 1;
    }

    if (textureBuffers.count(imageId) == 0)
    {
        // TODO: only load the diffuse image for now
        std::shared_ptr<VulkanImage> diffuseImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        std::shared_ptr<Image> imageToLoad = material->diffuseImage;
        diffuseImage->Load(
            std::vector<uint8_t *>({ imageToLoad->GetPixels() }),
            imageToLoad->GetWidth(),
            imageToLoad->GetHeight());

        std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>(
            context,
            descriptorPool,
            pipelines.staticPipeline->GetImageDescriptorSetLayout());
        texture->Create();
        texture->AddImage(diffuseImage, 0);
        texture->AddImage(cubeMapImage, 1);

        textureBuffers[imageId] = std::move(texture);
        textureBufferCount[imageId] = 1;
    }
    else
    {
        textureBufferCount[imageId] = textureBufferCount[imageId] + 1;
    }

    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
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
        instanceBuffers[instanceId].push_back(std::move(instanceBuffer));
    }

    VulkanEntityBufferIds bufferIds{};
    bufferIds.instanceBufferId = instanceId;
    bufferIds.vertexBufferId = meshId;
    bufferIds.indexBufferId = meshId;
    bufferIds.textureBufferId = imageId;
    bufferIdCache[instanceId] = bufferIds;

    VulkanEntityBuffer entityBuffer{};
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        entityBuffer.instanceBuffers.push_back(
            instanceBuffers[bufferIds.instanceBufferId][i].get());
    }
    entityBuffer.vertexBuffer = vertexBuffers[bufferIds.vertexBufferId].get();
    entityBuffer.indexBuffer = indexBuffers[bufferIds.indexBufferId].get();
    entityBuffer.textureBuffer = textureBuffers[bufferIds.textureBufferId].get();
    entityBufferCache[instanceId] = entityBuffer;
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
        screenObjectBuffers[screenObjectId] = std::move(vertexBuffer);

        std::shared_ptr<VulkanImage> screenImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        screenImage->Load(
            std::vector<uint8_t *>({ image->GetPixels() }),
            image->GetWidth(),
            image->GetHeight());

        std::unique_ptr<VulkanTexture> screenTexture = std::make_unique<VulkanTexture>(
            context,
            descriptorPool,
            pipelines.screenPipeline->GetImageDescriptorSetLayout());
        screenTexture->Create();
        screenTexture->AddImage(screenImage, 0);

        textureBuffers[screenObjectId] = std::move(screenTexture);
        textureBufferCount[screenObjectId] = 1;
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
    screenObjectBuffer.textureBuffer = textureBuffers[screenObjectId].get();
    screenObjectBufferCache[screenObjectId] = screenObjectBuffer;
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
        terrainVertexBuffers[terrainId] = std::move(vertexBuffer);

        std::shared_ptr<VulkanBuffer> indexBuffer = std::make_shared<VulkanBuffer>(
            context, commandPool, vmaAllocator);
        indexBuffer->Load(
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            indices.data(),
            static_cast<uint32_t>(sizeof(uint32_t) * indices.size()));
        terrainIndexBuffers[terrainId] = std::move(indexBuffer);

        std::shared_ptr<VulkanImage> terrainImage = std::make_shared<VulkanImage>(
            context, commandPool, imageVmaAllocator, VulkanImageType::Texture);
        terrainImage->Load(
            std::vector<uint8_t *>({ texture->GetPixels() }),
            texture->GetWidth(),
            texture->GetHeight());

        std::unique_ptr<VulkanTexture> terrainTexture = std::make_unique<VulkanTexture>(
            context,
            descriptorPool,
            pipelines.terrainPipeline->GetImageDescriptorSetLayout());
        terrainTexture->Create();
        terrainTexture->AddImage(terrainImage, 0);

        textureBuffers[terrainId] = std::move(terrainTexture);
        textureBufferCount[terrainId] = 1;
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
    terrainBuffer.textureBuffer = textureBuffers[terrainId].get();
    terrainBufferCache[terrainId] = terrainBuffer;
}

void VulkanBufferManager::ResetCommandPool(VkCommandPool commandPool)
{
    this->commandPool = commandPool;
}

void VulkanBufferManager::ResetScreenBuffers()
{
    int width = context->GetScreen()->GetWidth(),
        height = context->GetScreen()->GetHeight();
    screenBufferInput.screenWidth = static_cast<float>(width);
    screenBufferInput.screenHeight = static_cast<float>(height);
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
        std::vector<std::shared_ptr<VulkanBuffer>> &instanceBuffersToRemove = instanceBuffers[instanceBufferId];
        for (auto const &instanceBufferToRemove : instanceBuffersToRemove)
        {
            instanceBufferToRemove->Unload();
        }
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

        DestroyTextureBuffer(bufferIds.textureBufferId);

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

        DestroyTextureBuffer(screenObjectId);

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

        DestroyTextureBuffer(terrainId);

        terrainBufferCache.erase(terrainId);
    }
}

void VulkanBufferManager::UpdateInstanceBuffer(uint32_t instanceId, VulkanInstanceBufferInput &input, uint32_t imageIndex)
{
    if (instanceBuffers.count(instanceId) == 0)
    {
        return;
    }

    instanceBuffers[instanceId][imageIndex]->UpdateFast(&input, sizeof(VulkanInstanceBufferInput));
}

void VulkanBufferManager::UpdateUniformBuffer(VulkanUniformBufferInput &input, uint32_t imageIndex)
{
    uniformBuffers[imageIndex]->UpdateFast(&input, sizeof(VulkanUniformBufferInput));
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
    VulkanScreenBufferInput dummyScreenBufferInput{};
    for (uint32_t i = 0; i < frameBufferSize; i++)
    {
        std::unique_ptr<VulkanBuffer> screenBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
        screenBuffer->Load(
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &dummyScreenBufferInput,
            sizeof(VulkanScreenBufferInput));
        screenBuffer->CreateDescriptorSet(
            descriptorPool,
            pipelines.screenPipeline->GetUniformDescriptorSetLayout(),
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            sizeof(VulkanScreenBufferInput));
        screenBuffers.push_back(std::move(screenBuffer));
    }

    ResetScreenBuffers();
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
    if (!cubeMapBufferLoaded)
    {
        return;
    }

    cubeMapTexture->Destroy();
    cubeMapImage->Unload();
    cubeMapIndexBuffer->Unload();
    cubeMapVertexBuffer->Unload();
}

void VulkanBufferManager::DestroyScreenBuffers()
{
    for (std::unique_ptr<VulkanBuffer> &screenBuffer : screenBuffers)
    {
        screenBuffer->Unload();
    }
    screenBuffers.clear();
}

void VulkanBufferManager::DestroyUniformBuffers()
{
    for (std::unique_ptr<VulkanBuffer> &uniformBuffer : uniformBuffers)
    {
        uniformBuffer->Unload();
    }
    uniformBuffers.clear();
}

void VulkanBufferManager::DestroyTextureBuffer(uint32_t textureBufferId)
{
    textureBufferCount[textureBufferId] = textureBufferCount[textureBufferId] - 1;
    if (textureBufferCount[textureBufferId] == 0)
    {
        const auto &textureBuffer = textureBuffers[textureBufferId];
        const auto &images = textureBuffer->GetImages();
        for (const auto &image : images)
        {
            image->Unload();
        }
        textureBuffer->Destroy();
        textureBuffers.erase(textureBufferId);
    }
}
