#include "platform.h"
SCCSID(js_nativefile, "$Id: //depot/peter/w/js/win32/js_nativefile.c#1 $");

/*---------------------------------------------------------------------------
 * NAME
 *      js_nativefile - 
 *
 * SYNOPSIS
 */
INT
js_nativefile
(
    OUT     INT               * p_status
)
/*
 * DESCRIPTION
 * 
 * PARAMETERS
 * 
 * RETURN VALUES
 *
 * ERRORS
 *      RC_OK           Successful completion.
 *
 *---------------------------------------------------------------------------*/
{
    INT                 status          = RC_OK;
    INT                 result          = 0;
    /* end of local var. */

    /* parameter check */
    
    /* environment check */

    /* processing. */
    if ( status == RC_OK )
    {

    }

    /* return */
    if( p_status )
        *p_status = status;
    return result;
}

