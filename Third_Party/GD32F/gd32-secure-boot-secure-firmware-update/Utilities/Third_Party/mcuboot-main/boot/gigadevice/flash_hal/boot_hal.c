#include "bootutil/bootutil_log.h"

#if defined(__ARMCC_VERSION)
/* reimplement the function to reach Error Handler */
void __aeabi_assert(const char *expr, const char *file, int line)
{
    BOOT_LOG_ERR("assertion \" %s \" failed: file %s %d\n", expr, file, line);    
    while(1) {
    }
}
#endif  /*  __ARMCC_VERSION */
