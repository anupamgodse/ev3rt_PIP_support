#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>

uint32_t us_ticker_read() {
    SYSTIM systim;
    ER ercd = get_tim(&systim);
    assert(ercd == E_OK);
#if defined(BUILD_HRP2_LIBRARY)
    return systim * 1000UL;
#elif defined(BUILD_HRP3_LIBRARY)
    return systim;
#else
#error unsupported kernel version
#endif

}
