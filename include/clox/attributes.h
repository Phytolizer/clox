#ifndef ATTRIBUTES_H_
#define ATTRIBUTES_H_

#ifdef __GNUC__
#  define GNU_VERSION \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#  define GNU_VERSION 0
#endif

#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif

#if __has_attribute(nonnull) || defined(__GNUC__)
#  define ATTR_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#  define ATTR_ALL_NONNULL __attribute__((nonnull))
#else
#  define ATTR_NONNULL(...) /* non-null: __VA_ARGS__ */
#  define ATTR_ALL_NONNULL /* non-null: all */
#endif

#if __has_attribute(cleanup) || defined(__GNUC__)
#  define ATTR_CLEANUP(func) __attribute__((cleanup(func)))
#else
#  ifdef _MSC_VER
#    pragma warning \
        "Your compiler doesn't have the cleanup attribute. This program will probably leak memory."
#  else
#    pragma GCC warning \
        "Your compiler doesn't have the cleanup attribute. This program will probably leak memory."
#  endif
#  define ATTR_CLEANUP(func) /* leak memory */
#endif

#endif
