#include <unistd.h>     /* Sleep */
#include <stdio.h>      /* Printf */
/**
 * Start listening for any incoming connections
 * Params: Pointers to state that we poll for requests from the main thread
 * requestConnect -1 for when we are exiting cleanly
*/
void *startListening(void *vargp)
{
    for(;;)
    {
        sleep(5);
    }
}