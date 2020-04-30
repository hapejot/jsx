#include "platform.h"
SCCSID(WinMain, "$Id: //depot/peter/w/js/wmain.c#1 $");
#include "win.h"

/*---------------------------------------------------------------------------
 * NAME
 *      WinMain - This function is called by the system as the initial entry point for a Windows-based application. 
 *
 * SYNOPSIS
 */
int WINAPI WinMain(
    IN      HINSTANCE       hInstance, 
    IN      HINSTANCE       hPrevInstance, 
    IN      PSTR            szCmdLine, 
    IN      int             nCmdShow
)
/*
 * DESCRIPTION
 *
 * PARAMETERS
 *      hInstance 
 *          [in] Handle to the current instance of the application. 
 *      hPrevInstance
 *          [in] Handle to the previous instance of the application. This
 *          parameter is always NULL. If you need to detect whether another instance
 *          already exists, create a uniquely named mutex using the CreateMutex
 *          function. CreateMutex will succeed even if the mutex already exists,
 *          but the function will return ERROR_ALREADY_EXISTS. This indicates that
 *          another instance of your application exists, because it created the
 *          mutex first.
 *      lpCmdLine
 *          [in] Pointer to a null-terminated string
 *          specifying the command line for the application, excluding
 *          the program name. To retrieve the entire command line,
 *          use the GetCommandLine function.
 *      nCmdShow
 *          [in] Specifies how the window is to be
 *          shown. This parameter can be one of the following values.
 *
 *          SW_HIDE
 *              Hides the window and activates another window.
 *          SW_MAXIMIZE
 *              Maximizes the specified window.
 *          SW_MINIMIZE
 *              Minimizes the specified window and activates the next top-level window in the Z order.
 *          SW_RESTORE
 *              Activates and displays the window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when restoring a minimized window.
 *          SW_SHOW
 *              Activates the window and displays it in its current size and position. 
 *          SW_SHOWMAXIMIZED
 *              Activates the window and displays it as a maximized window.
 *          SW_SHOWMINIMIZED
 *              Activates the window and displays it as a minimized window.
 *          SW_SHOWMINNOACTIVE
 *              Displays the window as a minimized window. This value is similar to SW_SHOWMINIMIZED, except the window is not activated.
 *          SW_SHOWNA
 *              Displays the window in its current size and position. This value is similar to SW_SHOW, except the window is not activated.
 *          SW_SHOWNOACTIVATE
 *              Displays a window in its most recent size and position. This value is similar to SW_SHOWNORMAL, except the window is not actived.
 *          SW_SHOWNORMAL
 *              Activates and displays a window. If the window is minimized or maximized, the system restores it to its original size and position. An application should specify this flag when displaying the window for the first time.
 *
 * 
 * RETURN VALUES
 * If the function succeeds, terminating when it receives a WM_QUIT message, it should return the exit value contained in that message's wParam parameter. If the function terminates before entering the message loop, it should return zero. 
 *
 * ERRORS
 *      [RC_OK]         Successful completion.
 *
 *---------------------------------------------------------------------------*/
{
    INT rc = RC_OK;
    /* end of local var. */

    /* parameter check */
    
    /* environment check */

    tpc_queue_create( &g_queue );

    /* processing. */
    if ( rc == RC_OK )
    {
        HWND					hWnd;
 
        InitializeApplication();

        ghInstance = hInstance;


        net_init();
        net_connect( "localhost", "4200", &g_serv_connect );

        g_win_thread = CreateThread( 
                NULL,                   /* default security attributes  */
                0,                      /* default stack size           */
                win_work_thread,        /* start routine                */
                NULL,                   /* no parameters                */
                0,                      /* thread starts immediately    */
                &g_win_thread_id);

        assert( g_win_thread != 0 );

        CreateThread( 
                NULL,                   /* default security attributes  */
                0,                      /* default stack size           */
                win_serv_thread,        /* start routine                */
                NULL,                   /* no parameters                */
                0,                      /* thread starts immediately    */
                NULL);

        hWnd = CreateWindowEx( 
                0,                      /* no extended styles right now */
                "GenericFrame",         /* class name                   */
                "Generic Application",  /* window name              */
                WS_OVERLAPPEDWINDOW
                | WS_CLIPCHILDREN,      /* display features             */
                CW_USEDEFAULT,          /* x pos                        */
                CW_USEDEFAULT,          /* y pos                        */
                CW_USEDEFAULT,          /* width                        */
                CW_USEDEFAULT,          /* heigth                       */
                NULL,                   /* parent/owner window          */
                NULL,                   /* class menu                   */
                hInstance,              /* application instance         */
                NULL                    /* private data                 */
        );

        assert( hWnd );
        ShowWindow( hWnd, nCmdShow );
        UpdateWindow( hWnd );

        win_disp_thread(NULL);
    }

    /* return */
    return rc;
    szCmdLine;
    hPrevInstance;
}

