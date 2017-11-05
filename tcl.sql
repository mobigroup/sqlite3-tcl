.load ./libsqlitetcl.so

-- TCL (cmd, argv)
SELECT 'ERROR' WHERE TCL('return $argc','abba', 'baba') != '2';
SELECT 'ERROR' WHERE TCL('return $argv','abba', 'baba') != 'abba baba';
SELECT 'ERROR' WHERE TCL('lindex $argv 0','abba', 'baba') != 'abba';
SELECT 'ERROR' WHERE TCL('lindex $argv 1','abba', 'baba') != 'baba';
SELECT 'ERROR' WHERE TCL('dict get [lindex $argv 0] mykey','key 1 mykey 2') != '2';
SELECT 'ERROR' WHERE LENGTH(TCL('string repeat x 1024')) != 1024;

-- TCLCMD (cmd, args)
SELECT 'ERROR' WHERE TCLCMD('dict', 'get', 'key 1 mykey 2', 'mykey') != '2';
SELECT 'ERROR' WHERE TCLCMD('dict', 'get', 'key 1 mykey 2', 'key') != '1';
SELECT 'ERROR' WHERE TCLCMD('lindex', 'key 1 mykey 2', 0) != 'key';
SELECT 'ERROR' WHERE TCLCMD('lindex', 'key 1 mykey 2', 2) != 'mykey';

SELECT 'ERROR' WHERE TCLCMD('join', 'key 1 mykey 2', '-') != 'key-1-mykey-2';
SELECT 'ERROR' WHERE TCLCMD('lreplace', 'key 1 mykey 2', -1, -1, 'test') != 'test key 1 mykey 2';
SELECT 'ERROR' WHERE TCLCMD('lreplace', 'key 1 mykey 2', 'end', 'end', 'test') != 'key 1 mykey test';