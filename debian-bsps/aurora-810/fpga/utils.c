#include "utils.h"

#include <linux/mutex.h>
#include <linux/string.h>

struct mutex lock;

// Mutex lock
int sn_init_lock() {
    mutex_init(&lock);
    return SN_SUCCESS;
}
int sn_lock() {
    int retry = -LOCK_TIMECOUNT;
    while (retry) {
        if (!mutex_is_locked(&lock)) {
            mutex_lock(&lock);
            return SN_SUCCESS;
        } else {
            udelay(10);
            retry--;
        }
    }
    return -SN_LOCK_FAIL;
}
int sn_unlock() {
    mutex_unlock(&lock);
    return SN_SUCCESS;
}

// Utils
int bitmap_value_to_str(int bitmap_val, char *bitmap_str) {
    int i = 0;
    int tmp = 0;
    char buf[10] = {0};
    char tmp_buf[10] = {0};

    for (i = 0; i < 8; i++) {
        tmp = (0x01 << i);
        if (bitmap_val & tmp) {
            sprintf(tmp_buf, "1%s", buf);
        } else {
            sprintf(tmp_buf, "0%s", buf);
        }
        strcpy(buf, tmp_buf);
    }
    strcpy(bitmap_str, buf);
    return 0;
}

int s_atoi(const char *p) {
    int r = 0;
    int i = 0;
    int v, s = 1;

    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
        p += 2;

        while (*p) {
            i++;
            if (i > 8)
                break;

            if (*p >= '0' && *p <= '9')
                v = *p - '0';
            else if (*p >= 'a' && *p <= 'f')
                v = *p - 'a' + 10;
            else if (*p >= 'A' && *p <= 'F')
                v = *p - 'A' + 10;
            else
                break;

            r = r * 16 + v;
            p++;
        }
    } else {
        if (*p == '-') {
            s = -1;
            p++;
        }

        while (*p) {
            i++;
            if (i > 10)
                break;

            if (*p < '0' || *p > '9')
                break;

            r = r * 10 + (*p) - '0';
            p++;
        }
    }

    return r * s;
}

// TODO: migrate
static char *strtok_r(char *s, const char *delim, char **last) {
    char *spanp;
    int c, sc;
    char *tok;
    if (s == NULL && (s = *last) == NULL) {
        return NULL;
    }
cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc) {
            goto cont;
        }
    }
    if (c == 0) /* no non-delimiter characters */
    {
        *last = NULL;
        return NULL;
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0) {
                    s = NULL;
                } else {
                    char *w = s - 1;
                    *w = '\0';
                }
                *last = s;
                return tok;
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

char *strtok(char *s, const char *delim) {
    static char *last;
    return strtok_r(s, delim, &last);
}
