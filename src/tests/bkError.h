/******************************************************************************
* errnum.h
* this file contains #defined ints for return codes (errors and warnings)
* */

/* error codes in between these numbers */
#define BKERROR_MAX_ID                           -1001
#define BKERROR_MIN_ID                           -10000

/* warning codes in between these numbers */
#define BKWARNING_MAX_ID                         -10001
#define BKWARNING_MIN_ID                         -20000

/* a read() failed, no further info is available */
#define BKERROR_READ_GENERIC                     -1001
#define BKERROR_READ_GENERIC_TEXT                "Failed to read expected number of bytes"

/* do not make up #defines with numbers lower then this */
#define BKERROR_END                              -1000000
#define BKERROR_END_TEXT                         "Double oops, unusable error number used"

void outputError(int errorNum);
