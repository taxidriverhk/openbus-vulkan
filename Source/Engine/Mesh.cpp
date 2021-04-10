#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

MeshLoader::MeshLoader()
{
}

MeshLoader::~MeshLoader()
{
}

Mesh MeshLoader::LoadFromFile(const std::string filename)
{
    std::unordered_map<Vertex, uint32_t> uniqueVertices;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(
        filename,
        aiProcess_Triangulate);
    const uint32_t numMeshes = scene->mNumMeshes;
    aiMesh **meshes = scene->mMeshes;
    
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

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    Mesh result{};
    result.vertices = vertices;
    result.indices = indices;

    return result;
}
