#ifndef _COMMON_H_
#define _COMMON_H_

enum LOG_LEVEL {
    DBG  = 0x1,
    INFO = 0x2,
    ERR  = 0x4,
    ALL  = 0xf 
};

#define NB_LOG_ERR(_prefix, fmt, args...) \
            do { \
                if (loglv & ERR) \
                { \
                    printk( KERN_ERR _prefix " %s: "fmt, __FUNCTION__, ##args); \
                } \
            } while (0)

#define NB_LOG_INFO(_prefix, fmt, args...) \
            do { \
                if (loglv & INFO) \
                { \
                    printk( KERN_INFO _prefix " %s: "fmt, __FUNCTION__, ##args); \
                } \
            } while (0)

#define NB_LOG_DBG(_prefix, fmt, args...) \
            do { \
                if (loglv & DBG) \
                { \
                    printk( KERN_DEBUG _prefix " %s: "fmt, __FUNCTION__, ##args); \
                } \
            } while (0)


#define BITSET(byte,nbit)   ((byte) |=  (1<<(nbit)))
#define BITCLEAR(byte,nbit) ((byte) &= ~(1<<(nbit)))
#define BITFLIP(byte,nbit)  ((byte) ^=  (1<<(nbit)))
#define BITCHK(byte,nbit)   ((byte) &   (1<<(nbit)))


#endif /* _COMMON_H_ */
