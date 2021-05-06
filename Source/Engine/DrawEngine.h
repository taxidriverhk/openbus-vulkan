#pragma once

class Camera;
struct CubeMap;
struct Entity;
struct Terrain;
struct ScreenMesh;

class DrawEngine
{
public:
    virtual ~DrawEngine() {}

    virtual void Destroy() = 0;
    virtual void DrawFrame() = 0;
    virtual void Initialize() = 0;
    virtual void LoadCubeMap(CubeMap &cubemap) = 0;
    virtual void LoadEntity(Entity &entity) = 0;
    virtual void LoadScreenObject(ScreenMesh &screenMesh) = 0;
    virtual void LoadTerrain(Terrain &terrain) = 0;
    virtual void UpdateCamera(Camera *camera) = 0;
    virtual void UnloadEntity(uint32_t entityId) = 0;
    virtual void UnloadTerrain(uint32_t terrainId) = 0;
    virtual void UnloadScreenObject(uint32_t screenMeshId) = 0;
};
