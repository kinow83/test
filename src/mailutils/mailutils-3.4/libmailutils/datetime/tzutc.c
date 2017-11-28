#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <mailutils/datetime.h>

void
mu_datetime_tz_utc (struct mu_timezone *tz)
{
  tz->utc_offset = 0;
  tz->tz_name = "UTC";
}
