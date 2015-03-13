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

#define util_max(a, b) ((a) >= (b) ? (a) : (b))
#define util_min(a, b) ((a) >= (b) ? (b) : (a))
#define clearwctimer(tmr) (tmr = 0.0)
#define startwctimer(tmr) (tmr -= gk_WClockSeconds())
#define stopwctimer(tmr)  (tmr += gk_WClockSeconds())
#define getwctimer(tmr)   (tmr)

double wClockSeconds(void);
