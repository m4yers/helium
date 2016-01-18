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
#define LOG_ERROR 1
#define ERR(...) \
    fprintf(stderr, __VA_ARGS__);
#else
#define LOG_ERROR 0
#define ERR(...) (void) 0;
#endif

#if LOG_LEVEL > LOG_LEVEL_ERROR
#define LOG_WARNING 1
#define WRN(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define LOG_WARNING 0
#define WRN(...) (void) 0;
#endif

#if LOG_LEVEL > LOG_LEVEL_WARNING
#define LOG_INFO 1
#define NFO(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define LOG_INFO 0
#define NFO(...) (void) 0;
#endif

#if LOG_LEVEL > LOG_LEVEL_INFO
#define LOG_DEBUG 1
#define DBG(...) \
    fprintf(stdout, __VA_ARGS__);
#else
#define LOG_DEBUG 0
#define DBG(...) (void) 0;

#endif

static int blah;

#endif /* end of include guard: LOG_H_BLAKQPRH */
