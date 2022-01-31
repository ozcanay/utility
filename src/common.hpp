#pragma once

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define container_of(ptr, type, member) \
  ((type *) ((char *) (ptr) - offsetof(type, member)))

#define FUNC_NAME __PRETTY_FUNCTION__
#define HOT __attribute__((hot)) /// not sure about what this is.
#define UNUSED __attribute__((unused))
#define PREFETCH __builtin_prefetch

#define DISABLE_COMPILER_REORDERING __asm__ __volatile__("" ::: "memory"); /// important

#ifndef UNLIKELY
#define UNLIKELY(EXPR) __builtin_expect(!!(EXPR), 0) // !! is just a way of casting expression to bool, we might as well use static_cast<bool>.
#endif

#ifndef LIKELY
#define LIKELY(EXPR) __builtin_expect(!!(EXPR), 1)
#endif

#ifndef API_EXPORT
#define API_EXPORT __attribute__((visibility("default")))
#endif

#ifndef API_NO_EXPORT
#define API_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#ifndef API_DEPRECATED
#define API_DEPRECATED __attribute__((__deprecated__))
#endif
