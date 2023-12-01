#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Asks for user input from stdin with fallback
 * Handles printing newline characters
*/
void askQuestion(char *question, char *response, char *fallback, int maxLen)
{
    printf("%s", question);
    
    if (fgets(response, 40, stdin))
    {
        response[strcspn(response, "\n")] = 0;
    } else {
        response = fallback;
    }
}

/**
 * Asks for user input from stdin
 * Handles printing newline characters
*/
void askQuestionInt(char *question, int *response) 
{
    printf("%s", question);
    
    char temp[10];
    if (fgets(temp, 10, stdin))
    {
        temp[strcspn(temp, "\n")] = 0;
        *response = atoi(temp);
    } else {
        response = 0;
    }
}

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