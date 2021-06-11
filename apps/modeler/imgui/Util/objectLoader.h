#ifndef APPS_MODELER_UTIL_OBJECTLOADER
#define APPS_MODELER_UTIL_OBJECTLOADER

#include <Util/fileHandling.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/ECS/registry.h>

namespace Util {
  void loadOBJFile(Engine::Registry& registry, const fs::path& filePath);
}

#endif