#include <assert.h>

#define VMA_IMPLEMENTATION

#include "Engine/Vulkan/VulkanCommon.h"
#include "Engine/Vulkan/Command/VulkanDefaultRenderCommand.h"
#include "VulkanBufferManager.h"

VulkanBufferManager::VulkanBufferManager(
    VulkanContext *context,
    VulkanRenderPass *renderPass,
    VulkanDrawingPipelines pipelines,
    uint32_t frameBufferSize)
    : context(context),
      renderPass(renderPass),
      pipelines(pipelines),
      frameBufferSize(frameBufferSize),
      vmaAllocator(),
      imageVmaAllocator(),
      commandPool(),
      descriptorPool(),
      indexBuffers(),
      vertexBuffers(),
      uniformBufferUpdated(true),
      uniformBuffers(),
      uniformBufferInput(),
      cubeMapImage(),
      cubeMapIndexBuffer(),
      cubeMapVertexBuffer(),
      generator(),
      distribution(1, MAX_VERTEX_BUFFERS),
      drawingCommandCache{},
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

    CreateCommandPool();

    CreateUniformBuffers();
    CreateCubeMapBuffer();
}

void VulkanBufferManager::Destroy()
{
    DestroyUniformBuffers();
    DestroyCubeMapBuffer();

    // Destroying the descriptor pool will automatically free the descriptor sets
    // created using the pool, so no need to explicitly free each descriptor set
    vkDestroyDescriptorPool(context->GetLogicalDevice(), descriptorPool, nullptr);

    VkDevice logicalDevice = context->GetLogicalDevice();
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);

    vmaDestroyAllocator(vmaAllocator);
    vmaDestroyAllocator(imageVmaAllocator);
}

uint32_t VulkanBufferManager::LoadIntoBuffer(
    uint32_t meshId,
    uint32_t imageId,
    VulkanInstanceBufferInput &instanceBufferInput,
    std::vector<Vertex> &vertices,
    std::vector<uint32_t> &indices,
    Material *material)
{
    uint32_t bufferId = GenerateBufferId();

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
    instanceBuffers.insert(std::make_pair(bufferId, std::move(instanceBuffer)));

    VulkanDrawingBuffer drawingBuffer{};
    drawingBuffer.instanceBufferId = bufferId;
    drawingBuffer.vertexBufferId = meshId;
    drawingBuffer.indexBufferId = meshId;
    drawingBuffer.imageBufferId = imageId;
    drawingBuffers.insert(std::make_pair(bufferId, drawingBuffer));

    VulkanDrawingCommand drawingCommand{};
    drawingCommand.instanceBuffer = instanceBuffers[drawingBuffer.instanceBufferId].get();
    drawingCommand.vertexBuffer = vertexBuffers[drawingBuffer.vertexBufferId].get();
    drawingCommand.indexBuffer = indexBuffers[drawingBuffer.indexBufferId].get();
    drawingCommand.imageBuffer = imageBuffers[drawingBuffer.imageBufferId].get();
    drawingCommandCache.insert(std::make_pair(bufferId, drawingCommand));

    return bufferId;
}

void VulkanBufferManager::UnloadBuffer(uint32_t bufferId)
{
    if (drawingBuffers.count(bufferId) > 0)
    {
        VulkanDrawingBuffer drawingBuffer = drawingBuffers[bufferId];
        
        uint32_t instanceBufferId = drawingBuffer.instanceBufferId;
        std::shared_ptr<VulkanBuffer> instanceBuffer = instanceBuffers[instanceBufferId];
        instanceBuffer->Unload();
        instanceBuffers.erase(instanceBufferId);

        uint32_t vertexBufferId = drawingBuffer.vertexBufferId;
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

        uint32_t imageBufferId = drawingBuffer.imageBufferId;
        imageBufferCount[imageBufferId] = imageBufferCount[imageBufferId] - 1;
        if (imageBufferCount[imageBufferId] == 0)
        {
            std::shared_ptr<VulkanImage> imageBuffer = imageBuffers[imageBufferId];
            imageBuffer->Unload();
            imageBuffers.erase(imageBufferId);
        }

        drawingBuffers.erase(bufferId);
        drawingCommandCache.erase(bufferId);
    }
}

void VulkanBufferManager::UpdateCubeMapImage(std::vector<Image *> &images)
{
    assert(images.size() > 0);

    std::vector<uint8_t *> imagePixels;
    for (Image *image : images)
    {
        imagePixels.push_back(image->GetPixels());
    }
    cubeMapImage->UpdateImagePixels(
        imagePixels,
        images[0]->GetWidth(),
        images[0]->GetHeight());
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

void VulkanBufferManager::CreateCommandPool()
{
    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.queueFamilyIndex = context->GetGraphicsQueueIndex();
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    ASSERT_VK_RESULT_SUCCESS(
        vkCreateCommandPool(context->GetLogicalDevice(), &commandPoolInfo, nullptr, &commandPool),
        "Failed to create command pool");
}

void VulkanBufferManager::CreateCubeMapBuffer()
{
    // TODO: may consider loading the cube from a model file with configurable size multiplier
    const uint32_t cubeMapVertexCount = 8;
    std::array<Vertex, cubeMapVertexCount> cubeMapVertices;
    cubeMapVertices[0].position = { 1.0f, 1.0f, -1.0f };
    cubeMapVertices[1].position = { 1.0f, -1.0f, -1.0f };
    cubeMapVertices[2].position = { 1.0f, 1.0f, 1.0f };
    cubeMapVertices[3].position = { 1.0f, -1.0f, 1.0f };
    cubeMapVertices[4].position = { -1.0f, 1.0f, -1.0f };
    cubeMapVertices[5].position = { -1.0f, -1.0f, -1.0f };
    cubeMapVertices[6].position = { -1.0f, 1.0f, 1.0f };
    cubeMapVertices[7].position = { -1.0f, -1.0f, 1.0f };

    const float sizeMultiplier = 500.0f;
    for (uint32_t i = 0; i < cubeMapVertexCount; i++)
    {
        cubeMapVertices[i].position *= sizeMultiplier;
    }

    const uint32_t cubeMapIndexCount = 36;
    std::array<uint32_t, cubeMapIndexCount> cubeMapIndices =
    {
        4, 2, 0,
        2, 7, 3,
        6, 5, 7,
        1, 7, 5,
        0, 3, 1,
        4, 1, 5,
        4, 6, 2,
        2, 6, 7,
        6, 4, 5,
        1, 3, 7,
        0, 2, 3,
        4, 0, 1
    };

    cubeMapVertexBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
    cubeMapVertexBuffer->Load(
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        cubeMapVertices.data(),
        static_cast<uint32_t>(sizeof(Vertex) * cubeMapVertexCount));

    cubeMapIndexBuffer = std::make_unique<VulkanBuffer>(context, commandPool, vmaAllocator);
    cubeMapIndexBuffer->Load(
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        cubeMapIndices.data(),
        static_cast<uint32_t>(sizeof(uint32_t) * cubeMapIndexCount));

    // TODO: need to determine the image size more wisely
    cubeMapImage = std::make_unique<VulkanImage>(
        context, commandPool, imageVmaAllocator, VulkanImageType::CubeMap);
    cubeMapImage->Load(
        std::vector<uint8_t *>(),
        512,
        512,
        descriptorPool,
        pipelines.cubeMapPipeline->GetImageDescriptorSetLayout());

    cubeMapBufferCache.imageBuffer = cubeMapImage.get();
    cubeMapBufferCache.vertexBuffer = cubeMapVertexBuffer.get();
    cubeMapBufferCache.indexBuffer = cubeMapIndexBuffer.get();
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

uint32_t VulkanBufferManager::GenerateBufferId()
{
    uint32_t nextBufferId;
    do
    {
        nextBufferId = distribution(generator);
    } while (instanceBuffers.count(nextBufferId) != 0);
    return nextBufferId;
}
