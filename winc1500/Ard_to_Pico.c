
#include "Ard_to_Pico.h"
#include "pico/time.h"

uint32_t millis()
{
    absolute_time_t time_stamp = get_absolute_time();
    return to_ms_since_boot( time_stamp );
}

