#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

int main(int argc, const char *argv[]){
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc = 0;
    int col, cols, count;

    // open the database file
    rc = sqlite3_open("test.db", &db);
    if( rc ){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    sqlite3_enable_load_extension(db, 1);
    rc = sqlite3_exec(db, "SELECT LOAD_EXTENSION('/usr/lib/libsqlitetcl.so')", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %d : %s\n", rc, sqlite3_errmsg(db));
        return 1;
    }
    sqlite3_enable_load_extension(db, 0);

    // prepare the SQL statement
    rc = sqlite3_prepare_v2(db, "SELECT TCLCMD('lindex',list,1) FROM test limit 1", -1, &stmt, 0);
    if( rc ){
        fprintf(stderr, "SQL error: %d : %s\n", rc, sqlite3_errmsg(db));
    }else{
        cols = sqlite3_column_count(stmt);
        // execute the statement
        do{
            rc = sqlite3_step(stmt);
            switch( rc ){
            case SQLITE_DONE:
                break;
            case SQLITE_ROW:
                // print results for this row
                printf("%s\n", sqlite3_column_text(stmt,0));
                break;
            default:
                fprintf(stderr, "Error: %d : %s\n",  rc, sqlite3_errmsg(db));
                break;
            }
        }while( rc==SQLITE_ROW );
        // finalize the statement to release resources
        rc = sqlite3_finalize(stmt);
    }
    // close the database file
    sqlite3_close(db);

    return rc!=SQLITE_OK;
}
