#pragma once

inline constexpr int kMaxShaderProfiles = 3;

// Legacy compatibility.
// Keep this temporarily so old code can compile unchanged.
// Prefer kMaxShaderProfiles in new/refactored code.
#ifndef MAXSHADERPROFILE
#define MAXSHADERPROFILE kMaxShaderProfiles
#endif
