#pragma once

extern int backend_log_to_file(const char *path);
extern int backend_set_log_to_memory(void);
extern int backend_log(const char *format,...) __attribute__((format(printf,1,2)));
