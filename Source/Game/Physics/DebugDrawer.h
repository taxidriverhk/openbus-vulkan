#pragma once

#include <vector>

#include <LinearMath/btIDebugDraw.h>

#include "Engine/Vertex.h"

struct DebugSegments
{
    bool staticSegmentsUpdated;
    std::vector<LineSegmentVertex> staticSegments;
    std::vector<LineSegmentVertex> dynamicSegments;
};

class DebugDrawer : public btIDebugDraw
{
public:
    DebugDrawer();
    ~DebugDrawer();

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) override;
    void reportErrorWarning(const char *warningString) override;
    void draw3dText(const btVector3 &location, const char *textString) override;
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;

    void Clear();
    DebugSegments GetDrawingSegments() const;
    void SetIsDrawingStatic(bool isStatic);

private:
    static constexpr uint32_t MAX_SEGMENT_CAPACITY = 200000;

    bool isDrawingStatic;
    int debugMode;
    DebugSegments segments;
};
