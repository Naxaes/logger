#define LOGGER_IMPLEMENTATION
#include "logger.h"

#include <stdio.h>


int screaming_log(char* buffer, int size, const struct log_ctx_t* logger, log_level_t level, source_location_t source_location, const char* message, va_list args) {
    (void) logger;
    (void) level;
    (void) source_location;

    char new_message[1024] = { 0 };

    char* a = new_message;
    const char* b = message;
    char prev = '\0';
    do {
        char ch = *b;
        if ('a' <= ch && ch <= 'z' && prev != '%')
            *a = (char)(ch - 32);
        else
            *a = ch;

        prev = ch;
        ++a;
        ++b;
    } while (*b);


    int len = snprintf(buffer, size, "|%s| - ", log_level_name(level));
    len += vsnprintf(buffer + len, size - len, new_message, args);
    len += snprintf(buffer + len, size - len, "!!!");
    return len;
}


int other_api(int x) {
    trace("Entering '%s'", __func__);
    debug("Got parameter %d", x);
    info("Hello from other_api");
    warn("This api is under construction");
    return 0;
}

int main() {
    /* Initialize with explicit defaults
    log_init(
        .level = LOG_INFO,                  // Minimum log level
        .formatter = default_formatter,     // Default formatter for logs
        .sinks = {                          // Default sinks for each log level
            [LOG_TRACE]  = &stdout_sink,
            [LOG_DEBUG]  = &stdout_sink,
            [LOG_INFO]   = &stdout_sink,
            [LOG_WARN]   = &stderr_sink,
            [LOG_ERROR]  = &stderr_sink,
            [LOG_PANIC]  = &stderr_sink,
        },
        .disable_asserts = 0,
    );
    */
    /* Or let default initialization take place automatically (same as above) */

    printf("\n---------------------------------------- DEFAULT LOGGING ----------------------------------------\n");
    info("Hello world from main!");
    debug("This is a showcase of the logging capabilities");  /* Won't show as minimum default level is LOG_INFO */


    printf("\n---------------------------------------- PUSH CONFIGURABLE LOGGERS ---------------------------------------- \n");
    with_log(.level = LOG_TRACE, .sinks[LOG_WARN] = &stdout_sink, .sinks[LOG_ERROR] = &stdout_sink) {
        int result = other_api(10);
        error("Got %d from %s", result, "other_api");
    }
    error("Am know using defaults!");


    printf("\n---------------------------------------- MEMORY SINK ---------------------------------------- \n");
    char buffer[1024] = { 0 };
    struct ring_sink_state state = { .buffer = buffer, .size = sizeof(buffer) };
    log_sink_t mem_sink = log_sink_ring_buffer(&state);
    with_log(.name = "memory_sink_for_warn_and_error", .level = LOG_TRACE, .sinks[LOG_WARN] = &mem_sink, .sinks[LOG_ERROR] = &mem_sink) {
        printf(">>> Memory sink contains\n%s<<<\n", buffer);
        int result = other_api(10);
        printf(">>> Memory sink contains\n%s<<<\n", buffer);
        error("Got this '%d'", result);
        printf(">>> Memory sink contains\n%s<<<\n", buffer);
    }


    printf("\n---------------------------------------- CUSTOM FORMATTER ---------------------------------------- \n");
    with_log(.name = "custom_formatter", .formatter = screaming_log) {
        int result = other_api(10);
        error("Got %d from %s", result, "other_api");
    }


//    printf("\n---------------------------------------- ASSERTIONS ---------------------------------------- \n");
//    int a = 10;
//    int b = 0;
//    assert(a == b, "Whoops, lhs=%d and rhs=%d", a, b);

    return 0;
}
