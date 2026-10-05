// SPDX-License-Identifier: Apache-2.0
//

#include <TwkPaint/StampSeed.h>

namespace TwkPaint {

uint32_t seed_from_annotation_id(const char* id, size_t length)
{
    if (id == nullptr || length == 0) return 0;

    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i)
    {
        hash ^= static_cast<uint8_t>(id[i]);
        hash *= 16777619u;
    }
    return hash;
}

} // namespace TwkPaint
