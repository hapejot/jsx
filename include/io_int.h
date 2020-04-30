#ifndef IO_INT_H
#define IO_INT_H

#include "./io.h"

/*---------------------------------------------------------------------------
 * NAME
 *      io_int - Interne Deklarationen des IO Moduls.
 *
 * DESCRIPTION
 *
 *---------------------------------------------------------------------------*/

#define IO_UNGET_BUFSIZE        8

typedef struct IO_CLASS
{
    INT     (*read)     (IO_FILE, void*, INT, INT*);
    INT     (*write)    (IO_FILE, void*, INT, INT*);
    INT     (*seek)     (IO_FILE, UINT64);
    INT     (*tell)     (IO_FILE, UINT64*);
    INT     (*close)    (IO_FILE);
    INT     (*attr_get) (IO_FILE, char*, char*, INT );
}                   IO_CLASS;

struct IO_FILE
{
    IO_CLASS      * t;
    void          * p;
    INT             pint;
    CHAR            unget_buffer[IO_UNGET_BUFSIZE];
    INT             unget_count;
};

#ifndef GLOBALS
#endif

extern INT io_std_read      ( IO_FILE, void*, INT, INT* );
extern INT io_std_write     ( IO_FILE, void*, INT, INT* );
extern INT io_std_close     ( IO_FILE );
extern INT io_std_seek      ( IO_FILE, UINT64 );
extern INT io_std_attr_get  ( IO_FILE, char*, char*, INT );

extern INT io_buf_read      ( IO_FILE, void*, INT, INT* );
extern INT io_buf_write     ( IO_FILE, void*, INT, INT* );
#endif
