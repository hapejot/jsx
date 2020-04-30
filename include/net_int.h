#include "net.h"

extern INT net_read     (IO_FILE, void*, INT, INT*);
extern INT net_write    (IO_FILE, void*, INT, INT*);
extern INT net_close    ( IO_FILE );
extern INT net_attr_get ( IO_FILE, char*, char*, INT );
