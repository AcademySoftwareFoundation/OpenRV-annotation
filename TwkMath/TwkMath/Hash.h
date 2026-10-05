//******************************************************************************
//
// SPDX-License-Identifier: Apache-2.0
//
//******************************************************************************

#ifndef _TwkMathHash_h_
#define _TwkMathHash_h_

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace TwkMath
{

//*****************************************************************************
// hash_u32
//
// Deterministic, stable 32-bit hash of a byte string.
//
// Algorithm: FNV-1a 32-bit. Same input bytes always produce the same output
// value, on any platform, in any process — this makes it useful for deriving
// reproducible values (e.g. RNG seeds) from identifiers such as UUIDs.
//
// Returns 0 for a null pointer or zero-length input.
//*****************************************************************************
inline uint32_t hash_u32(const char* data, size_t length)
{
    if (data == nullptr || length == 0) return 0;

    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < length; ++i)
    {
        hash ^= static_cast<uint8_t>(data[i]);
        hash *= 16777619u;
    }
    return hash;
}

// Synonym for hash_u32(const char*, size_t) taking a std::string_view.
inline uint32_t hash_u32(std::string_view data)
{
    return hash_u32(data.data(), data.size());
}

} // namespace TwkMath

#endif
