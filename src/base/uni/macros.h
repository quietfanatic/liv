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

 // TODO: Remove these
#define AA(v) ::uni::require_throw(v)
#define DA(v) ::uni::expect(v)
