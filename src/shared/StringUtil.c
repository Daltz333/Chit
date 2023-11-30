#include <stdio.h>

/**
 * Returns 0 if input string is a valid IPv4 address
 * Credit: https://stackoverflow.com/a/792645
*/
int isValidIpAddress(char *str)
{
    int segs = 0;   /* Segment count. */
    int chcnt = 0;  /* Character count within segment. */
    int accum = 0;  /* Accumulator for segment. */

    /* Catch NULL pointer. */
    if (str == NULL)
        return 0;

    /* Process every character in string. */
    while (*str != '\0') 
    {
        /* Segment changeover. */

        if (*str == '.') {
            /* Must have some digits in segment. */
            if (chcnt == 0)
                return 0;

            /* Limit number of segments. */
            if (++segs == 4)
                return 0;

            /* Reset segment values and restart loop. */
            chcnt = accum = 0;
            str++;
            continue;
        }

        /* Check numeric. */
        if ((*str < '0') || (*str > '9'))
            return -1;

        /* Accumulate and check segment. */
        if ((accum = accum * 10 + *str - '0') > 255)
            return -1;

        /* Advance other segment specific stuff and continue loop. */
        chcnt++;
        str++;
    }

    /* Check enough segments and enough characters in last segment. */
    if (segs != 3)
        return -1;

    if (chcnt == 0)
        return -1;

    /* Address okay. */
    return 0;
}