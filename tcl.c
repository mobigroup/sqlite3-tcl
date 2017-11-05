/*
The extension coded by Alexey Pechnikov (pechnikov@mobigroup.ru) and tested on linux only.
The code is public domain.

Compile as
    //gcc -DUSE_TCL_STUBS -O3 -I/usr/include/tcl8.5 -fPIC -shared tcl.c -o libsqlitetcl.so -ltclstub8.5
    gcc -I/usr/include/tcl8.5 -fPIC -shared tcl.c -o libsqlitetcl.so -ltcl8.5

Note: we can create function for unload extension by Tcl_DeleteInterp(tcl_interp);

See more examples at http://sqlite.mobigroup.ru/wiki?name=ext_tcl

Usage

    .load ./libsqlitetcl.so
    -- TCL (cmd, argv)
    SELECT TCL('info patchlevel');
    8.5.8
    SELECT TCL('return $argc','abba', 'baba');
    2
    SELECT TCL('return $argv','abba', 'baba');
    abba baba
    SELECT TCL('lindex $argv 0','abba', 'baba');
    abba
    SELECT TCL('lindex $argv 1','abba', 'baba');
    baba
    SELECT TCL('dict get [lindex $argv 0] mykey','key 1 mykey 2');
    2

    -- TCLCMD (cmd, args)
    SELECT TCLCMD('dict', 'get', 'key 1 mykey 2', 'mykey');
    2
    SELECT TCLCMD('dict', 'get', 'key 1 mykey 2', 'key');
    1
    SELECT TCLCMD('lindex', 'key 1 mykey 2', 0);
    key
    SELECT TCLCMD('lindex', 'key 1 mykey 2', 2);
    mykey

*/

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_TCL)

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <tcl.h>

#ifndef SQLITE_CORE
  #include "sqlite3ext.h"
  SQLITE_EXTENSION_INIT1
#else
  #include "sqlite3.h"
#endif

Tcl_Interp *tcl_interp=NULL;

static void tcl_func(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    int i;
    Tcl_Obj *argsObj, *resultObj;

    if (argc < 1 || (sqlite3_value_type(argv[0]) != SQLITE_TEXT)) {
        sqlite3_result_error(context, "Function may have first argument with TCL script content", -1);
        return;
    }

    char zArgc[32];
    sqlite3_snprintf(sizeof(zArgc), zArgc, "%d", argc-1);
    Tcl_SetVar(tcl_interp,"argc", zArgc, TCL_GLOBAL_ONLY);
    
    argsObj = Tcl_NewObj();
    for (i = 1; i < argc; i++) {
        Tcl_ListObjAppendElement(tcl_interp, argsObj,
            Tcl_NewStringObj((char*)sqlite3_value_text(argv[i]), sqlite3_value_bytes(argv[i])));
    }
    Tcl_SetVar2Ex(tcl_interp, "argv", NULL, argsObj, 0);

    if (Tcl_Eval(tcl_interp, (char*)sqlite3_value_text(argv[0])) == TCL_ERROR) {
        sqlite3_result_error( context, Tcl_GetVar(tcl_interp, "errorInfo", 0), -1);
        return;
    }
    
    resultObj = Tcl_GetObjResult(tcl_interp);
    sqlite3_result_text( context, Tcl_GetString(resultObj), -1, SQLITE_TRANSIENT);
}

static void tcl_cmd_func(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    int i;
    Tcl_Obj *cmdObj, *resultObj;
    Tcl_CmdInfo info;
    char *errmsg;

    if (argc < 1 || (sqlite3_value_type(argv[0]) != SQLITE_TEXT)) {
        sqlite3_result_error(context, "Function may have one or more arguments", -1);
        return;
    }

    // check command name
    if (Tcl_GetCommandInfo(tcl_interp, (char*)sqlite3_value_text(argv[0]), &info) == 0) {
        errmsg = sqlite3_mprintf("Command '%s' does not exists", (char*)sqlite3_value_text(argv[0]));
        sqlite3_result_error(context, errmsg, -1);
        sqlite3_free(errmsg);
        return;    
    }
    cmdObj = Tcl_NewStringObj((char*)sqlite3_value_text(argv[0]), sqlite3_value_bytes(argv[0]));
    for (i = 1; i < argc; i++) {
        Tcl_ListObjAppendElement(tcl_interp, cmdObj,
            Tcl_NewStringObj((char*)sqlite3_value_text(argv[i]), sqlite3_value_bytes(argv[i])));
    }

    if (Tcl_EvalObjEx(tcl_interp, cmdObj, TCL_EVAL_DIRECT) == TCL_ERROR) {
        sqlite3_result_error( context, Tcl_GetVar(tcl_interp, "errorInfo", 0), -1);
        return;
    }
    
    resultObj = Tcl_GetObjResult(tcl_interp);
    sqlite3_result_text( context, Tcl_GetString(resultObj), -1, SQLITE_TRANSIENT);
}

int sqlite3TclInit(sqlite3 *db){	
    Tcl_FindExecutable("sqlite3");
    tcl_interp = Tcl_CreateInterp();
    //if (Tcl_Init(tcl_interp) != TCL_OK) {
     //   return SQLITE_ERROR;
    //}

    int rc;
    rc = sqlite3_create_function(db, "tcl",   -1, SQLITE_UTF8, 0, tcl_func, 0, 0);
    rc = sqlite3_create_function(db, "tclcmd", -1, SQLITE_UTF8, 0, tcl_cmd_func, 0, 0);
    return rc;
}

#if !SQLITE_CORE
int sqlite3_extension_init(
    sqlite3 *db, 
    char **pzErrMsg,
    const sqlite3_api_routines *pApi
){
    SQLITE_EXTENSION_INIT2(pApi)
    return sqlite3TclInit(db);
}
#endif

#endif
