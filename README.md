# sqlite3-tcl
Extension for TCL evaluation by user-defined function inside SQLite3 database

# About

I have a lot of TCL language structures stored in my databases and want to get access to these from sqlite3 shell.
So I wrote extension which is useful for testing and shell scripting.

# Compile

Build bedian package using debian/rules or compile as

```
gcc -I/usr/include/tcl8.5 -fPIC -shared tcl.c -o libsqlitetcl.so -ltcl8.5
```

# Simple Examples
```
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

    SELECT TCL('dict get [lindex $argv 0] mykey','key 1 mykey 2');
    2

    -- TCLCMD (cmd, args)
    SELECT TCLCMD('dict', 'get', 'key 1 mykey 2', 'mykey');
    2

    SELECT TCLCMD('lindex', 'key 1 mykey 2', 0);
    key

    SELECT TCLCMD('join', 'key 1 mykey 2', '-');
    key-1-mykey-2

    SELECT TCLCMD('lreplace', 'key 1 mykey 2', -1, -1, 'test');
    test key 1 mykey 2

    SELECT TCLCMD('lreplace', 'key 1 mykey 2', 'end', 'end', 'test');
    key 1 mykey test

    SELECT TCLCMD('linsert', 'a b', 0, 'c');
    c a b

    SELECT TCLCMD('linsert', 'a b', 'end', 'c');
    a b c

    SELECT TCLCMD('lsort', 'a c b');
    a b c

    SELECT TCLCMD('lsort', '-decreasing', 'a c b');
    c b a

    SELECT TCLCMD('lreverse', 'a c b');
    b c a

    SELECT TCLCMD('lsearch', 'a c b', 'b');
    2

    SELECT TCLCMD('lsearch', 'a c b', 'd');
    -1
```

# Complex Examples
```
SELECT
CASE
  WHEN TCLCMD('dict','exist','a 1 b 2 c 3', 'd')
    THEN TCLCMD('dict', 'get', 'a 1 b 2 c 3', 'd')
  ELSE ''
END;

SELECT
CASE
  WHEN TCLCMD('dict','exist','a 1 b 2 c 3', 'c')
    THEN TCLCMD('dict', 'get', 'a 1 b 2 c 3', 'c')
  ELSE ''
END;

3
```

# Shell Scripting

For autoload of the extension create SQLite resource file like to ~/.sqliterc
```.load /usr/lib/libsqlitetcl.so```
And you can use extension in shell scripting:
```
echo "SELECT TCLCMD('lindex', 'key 1 mykey 2', 2);" | sqlite3 
mykey
```

# Pure TCL Scripting

This extension is not needed for TCL scripts. We may define TCL proc "tclcmd" like to this example:
```
#!/usr/bin/tclsh8.5
package require sqlite3

sqlite3 db test.db
proc tclcmd {cmd args} {
    return [$cmd {*}$args]
}
db func tclcmd tclcmd

db eval {SELECT TCLCMD('lindex',list,1) as result FROM test limit 1} {
    puts $result
}

db close
```

# PostgreSQL Scripting

Variadic function arguments are not accessible from pltcl and so we use this stupid solution:
```
CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text)
  RETURNS text AS
$BODY$
  return [$1 $2]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text,
    arg3 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3 $4]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text,
    arg3 text, arg4 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3 $4 $5]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text,
    arg3 text, arg4 text, arg5 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3 $4 $5 $6]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text,
    arg3 text, arg4 text, arg5 text, arg6 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3 $4 $5 $6 $7]
$BODY$
  LANGUAGE pltcl IMMUTABLE;

CREATE OR REPLACE FUNCTION tclcmd(cmd text, arg1 text, arg2 text,
    arg3 text, arg4 text, arg5 text, arg6 text, arg7 text)
  RETURNS text AS
$BODY$
  return [$1 $2 $3 $4 $5 $6 $7 $8]
$BODY$
  LANGUAGE pltcl IMMUTABLE;
 ```
 
Well, now we can use Tcl commands as
```
select tclcmd('dict', 'get', '1 a 2 b 3 c', '3');
c
create table test(hash);
insert into test values ('a 1 b 2 c 3');
select tclcmd('dict','get', hash, 'b') from test;
2
```

# Performance tests

See folder "tests".

On Intel Core Quad 2.66 GHz processor I got these results:
```
$ ./make
select field by C application
a b c d e f

real	0m0.003s
user	0m0.000s
sys	0m0.000s
select list item from field by C application with TCL interp loaded into SQLite
b

real	0m0.004s
user	0m0.000s
sys	0m0.004s
select list item from field by TCL application
b

real	0m0.028s
user	0m0.020s
sys	0m0.004s
```

# History

The project moved from my own fossil repository.

