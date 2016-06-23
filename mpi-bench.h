
typedef double  data_t;

#define MALLOC(n,t) ((t *)malloc(n * sizeof(t)))
#define ALLOCA(n,t) ((t *)alloca(n * sizeof(t)))

#define DEBUG   1

#if DEBUG
#define DPRINTF(...)    printf(__VA_ARGS__)
#else
#define DPRINTF(...)
#endif

#if DEBUG
#define DDEF(c)    c
#else
#define DDEF(c)
#endif

#define HOST_NAME_LEN   128

/* Arbitrarily choose process 0 as the root process. */
#define RANK_ROOT   0

/*
 *  There won't be overlapping messages from the same process, so we use
 *  the same tag for all messages.
 */
#define TAG_GENERIC 0

/*
 *  Send a large message to test bandwidth accurately.  It should take
 *  a significant fraction of a second in order to drown out latency
 *  and time sampling error.
 */
#define MSG_SIZE            100*1024*1024
#define SMALL_MSG_COUNT     200

#define MS_PER_SEC_DBL      1000.0
#define MS_PER_SEC_LONG     1000L
#define MEG_DBL             1048576.0

#include "protos.h"

