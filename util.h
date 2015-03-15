#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <setjmp.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define clear_timer(t) (t = 0.0)
#define start_timer(t) (t -= wClockSeconds())
#define stop_timer(t)  (t += wClockSeconds())
#define get_timer(t)   (t)
#define util_max(a, b) ((a) >= (b) ? (a) : (b))
#define util_min(a, b) ((a) >= (b) ? (b) : (a))

double wClockSeconds(void);
