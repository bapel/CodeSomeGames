#pragma once

#include <stdint.h>
#include <EASTL\numeric_limits.h>

namespace ecs
{
    using EntityId = uint32_t;
    const auto InvalidEntityId = eastl::numeric_limits<EntityId>::max();

    using ComponentTypeId = uint32_t;
    using ComponentId = uint32_t;

    const int EntityNameLen = 64;
    const int MaxComponentsPerEntity = 16;
}