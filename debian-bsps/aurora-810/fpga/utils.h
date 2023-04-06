#ifndef _UTILS_H_
#define _UTILS_H_

#include <linux/delay.h>
#include <linux/printk.h>
#define PREFIX ""
#define FILENAME                                                         \
    ({                                                                   \
        const char *filename_start = __FILE__;                           \
        const char *filename = filename_start;                           \
        while (*filename != '\0')                                        \
            filename++;                                                  \
        while ((filename != filename_start) && (*(filename - 1) != '/')) \
            filename--;                                                  \
        filename;                                                        \
    })

#define PRINT(KERN_LVL, LVL, fmt, ...)                                                                     \
    if (SN_DEBUG && printk_ratelimit()) {                                                                  \
        _Pragma("GCC diagnostic ignored \"-Wformat-extra-args\"");                                         \
        printk(KERN_LVL PREFIX "[" LVL "][%s:%d - %s()] " fmt, FILENAME, __LINE__, __func__, __VA_ARGS__); \
    }
#define PRINT_ERR(...) PRINT(KERN_ERR, "ERR", __VA_ARGS__, "")
#define PRINT_INFO(...) PRINT(KERN_INFO, "INFO", __VA_ARGS__, "")
#define PRINT_DEBUG(...) PRINT(KERN_DEBUG, "DEBUG", __VA_ARGS__, "")
// lock
#define LOCK_TIMECOUNT 1000

// After errno.h
#define SN_DEBUG 0

#define SN_SUCCESS 0
#define SN_LOCK_FAIL 531
#define SN_FPGA_MMAP_ALLC_FAIL 532
#define SN_LED_WRITE_FAIL 533
#define SN_LED_READ_FAIL 534
#define SN_MEM_ERROR 535
#define SN_QSFP_READ_FAIL 536
#define SN_QSFP_WRITE_FAIL 537
#define SN_I2C_READ_FAIL 538
#define SN_I2C_WRITE_FAIL 539

// Mutex lock
int sn_init_lock(void);
int sn_lock(void);
int sn_unlock(void);

// Utils
int bitmap_value_to_str(int bitmap_val, char *bitmap_str);
char *strtok(char *s, const char *delim);
int s_atoi(const char *p);
#endif
