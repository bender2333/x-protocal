#ifndef __SYS_INIT__
#define __SYS_INIT__

typedef void (*sys_init)(void);

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    #define SECTION(x)                  __attribute__((section(x)))
#elif defined(__ICCARM__)
    #define SECTION(x)                  @x
#elif defined(__GNUC__)
    #define SECTION(x)                  __attribute__((section(x)))
#else
    #define SECTION(x)
#endif
            
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
    /* keil __attribute__((used)) make do not optimization */
    #define     SYS_INIT(func)                                                             \
                const sys_init                                                                  \
                sys_init_##func __attribute__((used)) SECTION("sys_init_sec") =                    \
                func;
#elif defined(__ICCARM__)
    /* IAR __root make do not optimization */
    #define     SYS_INIT(func)                                           \
                __root const sys_init                                         \
                sys_init_##func SECTION("sys_init_sec") =                        \
                func;
#elif defined(__GNUC__)
    #define     SYS_INIT(func)                                       \
                const sys_init                                            \
                sys_init_##func  __attribute__((used)) SECTION("sys_init_sec") =                    \
                func;
#else
    #define     SYS_INIT(func)
#endif

void sys_init_fun(void);

#endif /* __SYS_INIT__ */