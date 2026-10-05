// Deterministic seed derivation for annotation ids.
//
// Used to make stamp jitter reproducible across re-renders and apps when
// the same OTIO Paint.2 id (UUID) is supplied.
//
// Algorithm: FNV-1a 32-bit over UTF-8 bytes of the id string.
//
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace TwkPaint {

/// FNV-1a 32-bit hash of @a id bytes. Returns 0 for an empty string.
uint32_t seed_from_annotation_id(const char* id, size_t length);

inline uint32_t seed_from_annotation_id(std::string_view id)
{
    return seed_from_annotation_id(id.data(), id.size());
}

} // namespace TwkPaint
