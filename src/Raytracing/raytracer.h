#ifndef ENGINE_RAYTRACING_RAYTRACER
#define ENGINE_RAYTRACING_RAYTRACER

#include <vector>

namespace Engine
{
class Registry;

std::vector<float> raytraceScene(Registry &registry, int width, int height);

} // namespace Engine

#endif