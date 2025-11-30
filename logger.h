#ifndef _LOGGER_H
#define _LOGGER_H

#include <stddef.h>
#include <stdarg.h>


#ifdef assert
# error "'logger.h' defines a custom assert() macro and will shadow assert() from <assert.h>."
#endif


typedef enum log_level_t {
    LOG_DEFAULT,    /* Use the default log level defined by `log_init` */
    LOG_TRACE,      /* Logs useful for tracing execution through the call graph */
    LOG_DEBUG,      /* Logs useful for debugging */
    LOG_INFO,       /* Logs useful for general information */
    LOG_WARN,       /* Logs for potential errors or problems */
    LOG_ERROR,      /* Logs for actual errors that needs to be fixed */
    LOG_PANIC,      /* Logs a non-recoverable error and immediately kills the program */
    LOG_LEVEL_COUNT
} log_level_t;

const char* log_level_name(log_level_t level);


#define trace(...)  do { if (LOG_TRACE >= log_current->level) { logger_log(log_current, LOG_TRACE, current_source_location(), __VA_ARGS__); }} while (0)
#define debug(...)  do { if (LOG_DEBUG >= log_current->level) { logger_log(log_current, LOG_DEBUG, current_source_location(), __VA_ARGS__); }} while (0)
#define info(...)   do { if (LOG_INFO  >= log_current->level) { logger_log(log_current, LOG_INFO,  current_source_location(), __VA_ARGS__); }} while (0)
#define warn(...)   do { if (LOG_WARN  >= log_current->level) { logger_log(log_current, LOG_WARN,  current_source_location(), __VA_ARGS__); }} while (0)
#define error(...)  do { if (LOG_ERROR >= log_current->level) { logger_log(log_current, LOG_ERROR, current_source_location(), __VA_ARGS__); }} while (0)
#define panic(...)  do { if (LOG_PANIC >= log_current->level) { logger_log(log_current, LOG_PANIC, current_source_location(), __VA_ARGS__); terminate_with_backtrace(); }} while (0)

#define assertc(cond)      do { if (logger_assert_is_enabled() && !(cond)) { logger_assert_log(log_current, current_source_location(), #cond, "");          terminate_with_backtrace(); }} while (0)
#define assertf(cond, ...) do { if (logger_assert_is_enabled() && !(cond)) { logger_assert_log(log_current, current_source_location(), #cond, __VA_ARGS__); terminate_with_backtrace(); }} while (0)
#define assert(...)        SELECT_FUNCTION(assert, VA_ARGS_DISPATCH(__VA_ARGS__))(__VA_ARGS__)


#define trace_loc(loc, ...)  do { if (LOG_TRACE >= log_current->level) { logger_log(log_current, LOG_TRACE, loc, __VA_ARGS__); }} while (0)
#define debug_loc(loc, ...)  do { if (LOG_DEBUG >= log_current->level) { logger_log(log_current, LOG_DEBUG, loc, __VA_ARGS__); }} while (0)
#define info_loc(loc, ...)   do { if (LOG_INFO  >= log_current->level) { logger_log(log_current, LOG_INFO,  loc, __VA_ARGS__); }} while (0)
#define warn_loc(loc, ...)   do { if (LOG_WARN  >= log_current->level) { logger_log(log_current, LOG_WARN,  loc, __VA_ARGS__); }} while (0)
#define error_loc(loc, ...)  do { if (LOG_ERROR >= log_current->level) { logger_log(log_current, LOG_ERROR, loc, __VA_ARGS__); }} while (0)
#define panic_loc(loc, ...)  do { if (LOG_PANIC >= log_current->level) { logger_log(log_current, LOG_PANIC, loc, __VA_ARGS__); terminate_with_backtrace(); }} while (0)

#define assert_locc(loc, cond)      do { if (logger_assert_is_enabled() && !(cond)) { logger_assert_log(log_current, loc, #cond, "");          terminate_with_backtrace(); }} while (0)
#define assert_locf(loc, cond, ...) do { if (logger_assert_is_enabled() && !(cond)) { logger_assert_log(log_current, loc, #cond, __VA_ARGS__); terminate_with_backtrace(); }} while (0)
#define assert_loc(loc, ...)        SELECT_FUNCTION(assert_loc, VA_ARGS_DISPATCH(__VA_ARGS__))(loc, __VA_ARGS__)


typedef struct source_location_t {
    const char* file;
    const char* function;
    int line;
} source_location_t;

#define current_source_location() ((source_location_t) { __FILE__, __func__, __LINE__ })
#define no_source_location()      ((source_location_t) { "", "", 0 })



typedef struct log_record_t {
    const char*         logger_name;
    log_level_t         level;
    source_location_t   location;
    const char*         message;
    size_t              message_len;
} log_record_t;


typedef void (*log_sink_write_fn)(void* data, const log_record_t* record);
typedef struct log_sink_t {
    log_sink_write_fn write;
    void* data;
} log_sink_t;


struct log_ctx_t;
typedef int (*log_formatter_fn)(char* buffer, int size, const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args);

typedef struct log_ctx_t {
    const char* name;
    log_level_t level;
    log_formatter_fn formatter;
    struct log_sink_t* sinks[LOG_LEVEL_COUNT];
    struct log_ctx_t* parent;
    source_location_t declaration_location;
} log_ctx_t;


#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
# define THREAD_LOCAL _Thread_local
#elif defined(_MSC_VER)
# define THREAD_LOCAL __declspec(thread)
#else
# define THREAD_LOCAL __thread
#endif


// INVARIANT: log_current is never NULL. Do not assign to it directly.
extern THREAD_LOCAL struct log_ctx_t* log_current;


static inline log_sink_t*       get_global_sink(log_level_t level);
static inline log_level_t       get_global_log_level(void);
static inline log_formatter_fn  get_global_log_formatter(void);

typedef void (*logger_log_fn)(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args);
void logger_log_impl(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args);
void logger_log_init(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args);
extern logger_log_fn logger_log_internal;


__attribute__((format(printf, 4, 5), always_inline))
static inline void logger_log(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, ...) {
    va_list args;
    va_start(args, message);
    logger_log_internal(logger, level, source_location, message, args);
    va_end(args);
}

__attribute__((format(printf, 4, 5)))
void logger_assert_log(const struct log_ctx_t* logger, source_location_t source_location, const char* condition, const char* message, ...);


__attribute__((noinline, noreturn, cold))
void terminate_with_backtrace(void);


#define INTERNAL_CONCAT_HELP(x, y)  x ## y
#define INTERNAL_CONCATENATE(x, y)  INTERNAL_CONCAT_HELP(x, y)
#define POP_LEFT_VA_ARGS_(x1, x2, x3, x4, x5, x6, x7, x8, xN, ...) xN
#define VA_ARGS_DISPATCH(...)  POP_LEFT_VA_ARGS_(__VA_ARGS__, f, f, f, f, f, f, f, c, c)
#define SELECT_FUNCTION(function, postfix) INTERNAL_CONCATENATE(function, postfix)

#if (defined(__has_attribute) && __has_attribute(cleanup)) || (defined(__GNUC__) && !defined(__clang__))
static inline void logger__missing_pop_error(struct log_ctx_t* log) {
    if (log->parent != NULL) { panic_loc(log->declaration_location, "Logger '%s' wasn't popped!", log->name); }
}
#  define LOGGER_ATTRIBUTE_CLEANUP __attribute__((cleanup(logger__missing_pop_error)))
#else
#  define LOGGER_ATTRIBUTE_CLEANUP
#endif

#define with_log(...) for ( \
    LOGGER_ATTRIBUTE_CLEANUP struct log_ctx_t _log = { __VA_ARGS__, .parent = log_current, .declaration_location = current_source_location() };     \
    _log.parent != 0 && (logger_push(&_log, current_source_location()), 1);                                                                         \
    logger_pop(&_log, current_source_location())                                                                                                    \
)

void logger_push(struct log_ctx_t* log, source_location_t location);
void logger_pop(struct log_ctx_t* log, source_location_t location);


extern log_sink_t stdout_sink;
extern log_sink_t stderr_sink;

int logger_assert_is_enabled(void);
log_sink_t log_sink_ring_buffer(char *buffer, size_t size);


typedef struct log_init_args_t {
    int _sentinel;
    log_formatter_fn formatter;
    log_level_t level;
    log_sink_t* sinks[LOG_LEVEL_COUNT];
    int disable_asserts;
} log_init_args_t;

#define log_init(...) log_init_from_args((log_init_args_t){ 0, __VA_ARGS__ })

void log_init_from_args(log_init_args_t args);

#endif  // _LOGGER_H




#ifdef LOGGER_IMPLEMENTATION

#include <stdlib.h>     // free, exit, EXIT_FAILURE
#include <stdio.h>      // fprintf, stderr
#include <execinfo.h>   // backtrace, backtrace_symbols
#include <signal.h>     // signal, SIGTRAP
#include <unistd.h>
#include <string.h>


static void fd_sink_write(void* data, const struct log_record_t* rec) {
    int fd = (int)(uintptr_t)data;
    if (fd < 0) return;

    // best-effort write, ignore errors for now
    (void) write(fd, rec->message, rec->message_len);
    (void) write(fd, "\n", 1);
}

log_sink_t log_sink_from_fd(int fd) {
    log_sink_t sink = {
            .write = fd_sink_write,
            .data = (void*) (uintptr_t) fd
    };
    return sink;
}


struct ring_sink_state {
    char  *buf;
    size_t size;
    size_t head;
};

static void ring_sink_write(void* data, const struct log_record_t* record) {
    struct log_record_t rec = *record;
    struct ring_sink_state* st = data;

    if (!st || !st->buf || st->size == 0) return;

    size_t want = rec.message_len + 2; // + '\n' + '\0'
    if (want > st->size) {
        // Truncate: keep last size-1 chars
        rec.message += want - (st->size - 1);
        rec.message_len = st->size - 1;
        want = st->size;
    }

    if (st->head + want > st->size) {
        st->head = 0;
    }

    if (st->head > 0) {
        st->head -= 1;  // Remove '\0' from previous message.
    }

    memcpy(st->buf + st->head, rec.message, rec.message_len);
    st->head += rec.message_len;
    st->buf[st->head++] = '\n';
    st->buf[st->head++] = '\0';
    if (st->head >= st->size) {
        st->head = 0;
    }
}

log_sink_t log_sink_ring_buffer(char *buffer, size_t size) {
    struct ring_sink_state* st = malloc(sizeof(struct ring_sink_state));

    st->buf  = buffer;
    st->size = size;
    st->head = 0;

    log_sink_t sink = {
            .write = ring_sink_write,
            .data  = st
    };

    return sink;
}





__attribute__((noinline, noreturn, cold))
void terminate_with_backtrace(void) {
    void* callstack[128] = { 0 };
    int frames = backtrace(callstack, 128);
    char** strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames-1; ++i) {
        fprintf(stderr, "%s\n", strs[i]);
    }
    free(strs);
    abort();
}

// INVARIANT: log_current is never NULL. Do not assign to it directly.
static struct log_ctx_t log_default = {
        .name = "",
        .level = LOG_DEFAULT,
        .formatter = NULL,
        .sinks = { 0 },
        .parent = NULL,
        .declaration_location = { 0 },
};
THREAD_LOCAL struct log_ctx_t* log_current = &log_default;



void logger_push(struct log_ctx_t* log, source_location_t location) {
    log->parent = log_current;
    log->level  = (log->level == LOG_DEFAULT) ? get_global_log_level() : log->level;

    log_current = log;
    source_location_t loc = log->declaration_location;
    trace_loc(location, "Pushed log declared at %s:%d", loc.file, loc.line);
}

void logger_pop(struct log_ctx_t* log, source_location_t location) {
    if (log_current != log) {
        panic_loc(location, "Cannot pop log!");
    }

    source_location_t loc = log->declaration_location;
    trace_loc(location, "Popped log declared at %s:%d", loc.file, loc.line);

    log_current = log_current->parent;

    log->name = "";
    log->level = LOG_DEFAULT;
    log->parent = NULL;
    log->declaration_location = no_source_location();
}

void logger_assert_log(const struct log_ctx_t* logger, source_location_t source_location, const char* condition, const char* message, ...) {
    char buffer[1024];
    int len = snprintf(buffer, sizeof buffer, "'%s' failed.", condition);
    if (len < 0) len = 0;
    if (len >= (int)sizeof buffer) len = (int)sizeof buffer - 1;

    if (message && message[0] != '\0') {
        if (len < (int)sizeof(buffer)) {
            buffer[len++] = ' ';
        }
        va_list args;
        va_start(args, message);
        vsnprintf(buffer + len, sizeof(buffer) - len, message, args);
        va_end(args);
    }

    logger_log(logger, LOG_PANIC, source_location, "%s", buffer);
}


static const char* LOG_LEVEL_NAMES[LOG_LEVEL_COUNT] = {
        [LOG_DEFAULT] = "default",
        [LOG_TRACE]   = "trace",
        [LOG_DEBUG]   = "debug",
        [LOG_INFO]    = "info",
        [LOG_WARN]    = "warn",
        [LOG_ERROR]   = "error",
        [LOG_PANIC]   = "panic",
};

const char* log_level_name(log_level_t level) {
    return LOG_LEVEL_NAMES[level];
}



int default_formatter(char* buffer, int size, const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args) {
    const char* file = source_location.file ? source_location.file : "?";
    const char* name = logger->name ? logger->name : "";

    int header_len = *name
            ? snprintf(buffer, size-1, "%s:%-3d [%s:%s]: ", file, source_location.line, name, LOG_LEVEL_NAMES[level])
            : snprintf(buffer, size-1, "%s:%-3d [%s]: ",    file, source_location.line, LOG_LEVEL_NAMES[level]);

    if (header_len < 0)
        header_len = 0;
    if (header_len >= size)
        header_len = size - 1;

    int body_len = vsnprintf(buffer + header_len, size - header_len - 1, message, args);

    if (body_len < 0)
        body_len = 0;
    if (header_len + body_len >= size)
        body_len = size - header_len - 1;

    return header_len + body_len;
}


logger_log_fn logger_log_internal = logger_log_init;

static int assert_is_enabled = 1;
static log_formatter_fn global_log_formatter;
static log_level_t global_log_level;
static log_sink_t* global_sinks[LOG_LEVEL_COUNT];

log_sink_t stdout_sink;
log_sink_t stderr_sink;

int logger_assert_is_enabled(void) {
    return assert_is_enabled;
}

static inline log_level_t       get_global_log_level(void)              { return global_log_level; }
static inline log_formatter_fn  get_global_log_formatter(void)          { return global_log_formatter; }
static inline log_sink_t*       get_global_sink(log_level_t level)      { return global_sinks[level]; }


void log_init_from_args(log_init_args_t args) {
    static int initialized = 0;
    if (initialized) {
        return;
    }
    initialized = 1;

    stdout_sink = log_sink_from_fd(STDOUT_FILENO);
    stderr_sink = log_sink_from_fd(STDERR_FILENO);

    global_log_formatter = (args.formatter == NULL) ? default_formatter : args.formatter;
    global_log_level = (args.level == LOG_DEFAULT) ? LOG_INFO : args.level;

    log_default.level = global_log_level;

    global_sinks[LOG_TRACE]  = (args.sinks[LOG_TRACE]  == NULL) ? &stdout_sink : args.sinks[LOG_TRACE];
    global_sinks[LOG_DEBUG]  = (args.sinks[LOG_DEBUG]  == NULL) ? &stdout_sink : args.sinks[LOG_DEBUG];
    global_sinks[LOG_INFO]   = (args.sinks[LOG_INFO]   == NULL) ? &stdout_sink : args.sinks[LOG_INFO];
    global_sinks[LOG_WARN]   = (args.sinks[LOG_WARN]   == NULL) ? &stderr_sink : args.sinks[LOG_WARN];
    global_sinks[LOG_ERROR]  = (args.sinks[LOG_ERROR]  == NULL) ? &stderr_sink : args.sinks[LOG_ERROR];
    global_sinks[LOG_PANIC]  = (args.sinks[LOG_PANIC]  == NULL) ? &stderr_sink : args.sinks[LOG_PANIC];

    assert_is_enabled = !args.disable_asserts;
}


__attribute__((noinline, cold))
void logger_log_init(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args) {
    log_init();
    logger_log_internal = logger_log_impl;
    logger_log_impl(logger, level, source_location, message, args);
}

void logger_log_impl(const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args) {
    char buffer[1024] = { 0 };
    int total_len;

    log_formatter_fn formatter = logger->formatter;
    if (!formatter)
        formatter = get_global_log_formatter();
    if (!formatter)
        formatter = default_formatter;

    total_len = formatter(buffer, (int) sizeof(buffer), logger, level, source_location, message, args);

    if (total_len < 0)
        total_len = 0;

    if (total_len >= (int) sizeof(buffer))
        total_len = (int) sizeof(buffer) - 1;

    struct log_record_t record = {
            .logger_name = logger->name,
            .level       = level,
            .location    = source_location,
            .message     = buffer,
            .message_len = (size_t)total_len,
    };

    log_sink_t* sink = logger->sinks[level];
    if (!sink)
        sink = get_global_sink(level);
    if (sink && sink->write) {
        sink->write(sink->data, &record);
    }
}


#endif  // LOGGER_IMPLEMENTATION
