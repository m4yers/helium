#ifndef LOG_H_BLAKQPRH
#define LOG_H_BLAKQPRH

#define LOG_LEVEL_NONE 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_ERROR
#endif

#if LOG_LEVEL > LOG_LEVEL_NONE

#define ERR(...) \
    fprintf(stderr, __VA_ARGS__);
#else
#define ERR(...) (void) 0
#endif

#if LOG_LEVEL > LOG_LEVEL_ERROR
#define WRN(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define WRN(...) (void) 0
#endif

#if LOG_LEVEL > LOG_LEVEL_WARNING
#define NFO(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define NFO(...) (void) 0
#endif

#if LOG_LEVEL > LOG_LEVEL_INFO
#define DBG(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define DBG(...) (void) 0

#endif

#endif /* end of include guard: LOG_H_BLAKQPRH */
