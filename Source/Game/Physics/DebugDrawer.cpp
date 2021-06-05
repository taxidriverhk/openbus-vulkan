#include "Common/Logger.h"
#include "DebugDrawer.h"

DebugDrawer::DebugDrawer()
    : debugMode(btIDebugDraw::DBG_DrawWireframe),
      isDrawingStatic(false),
      segments{}
{
    segments.staticSegments.reserve(MAX_SEGMENT_CAPACITY);
    segments.dynamicSegments.reserve(MAX_SEGMENT_CAPACITY);
}

DebugDrawer::~DebugDrawer()
{
}

void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    LineSegmentVertex v1, v2;
    v1.color = { color.x(), color.y(), color.z() };
    v1.position = { from.x(), from.y(), from.z() };
    v2.color = { color.x(), color.y(), color.z() };
    v2.position = { to.x(), to.y(), to.z() };

    if (isDrawingStatic)
    {
        segments.staticSegments.push_back(v1);
        segments.staticSegments.push_back(v2);
    }
    else
    {
        segments.dynamicSegments.push_back(v1);
        segments.dynamicSegments.push_back(v2);
    }
}

void DebugDrawer::drawContactPoint(const btVector3 &pointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{
    btVector3 from = pointOnB;
    btVector3 to = pointOnB + normalOnB * 1;

    LineSegmentVertex v1, v2;
    v1.color = { color.x(), color.y(), color.z() };
    v1.position = { from.x(), from.y(), from.z() };
    v2.color = { color.x(), color.y(), color.z() };
    v2.position = { to.x(), to.y(), to.z() };

    if (isDrawingStatic)
    {
        segments.staticSegments.push_back(v1);
        segments.staticSegments.push_back(v2);
    }
    else
    {
        segments.dynamicSegments.push_back(v1);
        segments.dynamicSegments.push_back(v2);
    }
}

void DebugDrawer::reportErrorWarning(const char *warningString)
{
    Logger::Log(LogLevel::Warning, warningString);
}

void DebugDrawer::draw3dText(const btVector3 &location, const char *textString)
{
    // TODO: not supported at this time
}

void DebugDrawer::setDebugMode(int debugMode)
{
    this->debugMode = debugMode;
}

int DebugDrawer::getDebugMode() const
{
    return debugMode;
}

// Should be called before debug drawing the next frame to clear any old line segments
void DebugDrawer::Clear()
{
    if (isDrawingStatic)
    {
        segments.staticSegmentsUpdated = true;
        segments.staticSegments.clear();
    }
    else
    {
        segments.staticSegmentsUpdated = false;
        segments.dynamicSegments.clear();
    }
}

DebugSegments DebugDrawer::GetDrawingSegments() const
{
    return segments;
}

void DebugDrawer::SetIsDrawingStatic(bool isStatic)
{
    isDrawingStatic = isStatic;
}
