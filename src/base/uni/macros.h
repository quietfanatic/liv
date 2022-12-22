 // This file has a number of convenient macros, which we're keeping separate
 // to avoid potential conflicts.

#pragma once

#define CE constexpr

#ifdef __has_builtin
#define HAS_BUILTIN(f) __has_builtin(f)
#else
#define HAS_BUILTIN(f)
#endif

 // Recommended that you make sure the move and copy constructors can't throw,
 // or else the assignee may be left destructed.
#define ASSIGN_BY_MOVE(T) \
T& operator = (T&& o) noexcept { \
    this->~T(); \
    return *new (this) T (std::move(o)); \
}
#define ASSIGN_BY_COPY(T) \
T& operator = (const T& o) noexcept { \
    this->~T(); \
    return *new (this) T (o); \
}

#define AA(v) ::uni::assert_general(v, __FUNCTION__, __FILE__, __LINE__)
#define DA(v) ::uni::debug_assert(v)
