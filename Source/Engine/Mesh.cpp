#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Common/Logger.h"
#include "Mesh.h"

MeshLoader::MeshLoader()
{
}

MeshLoader::~MeshLoader()
{
}

bool MeshLoader::LoadFromFile(const std::string filename, Mesh &mesh)
{
    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices);
    if (scene == nullptr)
    {
        const char *importError = importer.GetErrorString();
        Logger::Log(LogLevel::Error, "Failed to load mesh from file %s: %s", filename.c_str(), importError);
        return false;
    }

    const uint32_t numMeshes = scene->mNumMeshes;
    aiMesh **meshes = scene->mMeshes;
    
    uint32_t indexOffset = 0;

    for (uint32_t m = 0; m < numMeshes; m++)
    {
        const aiMesh *mesh = meshes[m];

        uint32_t numVertices = mesh->mNumVertices;
        const aiVector3D *texCoords = mesh->mTextureCoords[0];
        const aiVector3D *importedVertices = mesh->mVertices;
        const aiVector3D *importedNormals = mesh->mNormals;

        for (uint32_t i = 0; i < numVertices; i++)
        {
            Vertex vertex{};
            vertex.position.x = importedVertices[i].x;
            vertex.position.y = importedVertices[i].y;
            vertex.position.z = importedVertices[i].z;
            vertex.normal.x = importedNormals[i].x;
            vertex.normal.y = importedNormals[i].y;
            vertex.normal.z = importedNormals[i].z;
            vertex.uv.x = texCoords[i].x;
            vertex.uv.y = texCoords[i].y;
            vertices.push_back(vertex);
        }

        uint32_t numFaces = mesh->mNumFaces;
        const aiFace *faces = mesh->mFaces;
        for (uint32_t i = 0; i < numFaces; i++)
        {
            uint32_t numIndices = faces[i].mNumIndices;
            const uint32_t *importedIndices = faces[i].mIndices;
            for (uint32_t j = 0; j < numIndices; j++)
            {
                indices.push_back(importedIndices[j] + indexOffset);
            }
        }

        indexOffset += numVertices;
    } 

    mesh.vertices = vertices;
    mesh.indices = indices;

    return true;
}
