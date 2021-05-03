#include "SimpleRV32I_utils.h"
//a super dumb version of debug printing using environment variable
void debug_printf(uint8_t debugLevel, const char * format, ... ) {
    va_list ap;
    uint8_t debug_print_en = (getenv("DEBUG") != NULL) && atoi((getenv("DEBUG")));
    uint8_t debug_level_cmd = debug_print_en ? atoi(getenv("DEBUG")) : 0;
    va_start(ap,format);
    if(debug_print_en & (debugLevel <= debug_level_cmd)) vprintf(format,ap);
    va_end(ap);
}
