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
