 // This file has a number of convenient macros, which we're keeping separate
 // to avoid potential conflicts.

#pragma once

#define CE constexpr

 // Recommended that you make sure the move and copy constructors can't throw,
 // or else the assignee may be left destructed.
#define ASSIGN_BY_MOVE(T) \
T& operator = (T&& o) { \
    this->~T(); \
    new (this) T (std::move(o)); \
    return *this; \
}
#define ASSIGN_BY_COPY(T) \
T& operator = (const T& o) { \
    this->~T(); \
    new (this) T (o); \
    return *this; \
}

#define AA(v) ::uni::assert_general(v, __FUNCTION__, __FILE__, __LINE__)
#ifdef NDEBUG
#define DA(v)
#else
#define DA(v) AA(v)
#endif
#define AS(v) ::uni::assert_sdl(v, __FUNCTION__, __FILE__, __LINE__)
