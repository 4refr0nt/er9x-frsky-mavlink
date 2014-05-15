#include "er9x.h"
#include "stamp-er9x.h"

#define STR2(s) #s
#define DEFNUMSTR(s)  STR2(s)

const char APM  stamp1[] = "VERS: V" DEFNUMSTR(VERS) "." DEFNUMSTR(SUB_VERS);
const char APM  stamp2[] = "DATE: " DATE_STR;
const char APM  stamp3[] = "TIME: " TIME_STR;
const char APM  stamp4[] = " SVN: " SVN_VERS;
const char APM  stamp5[] = " MOD: " MOD_VERS;

