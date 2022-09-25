
* READ UNCOMMITTED(커밋되지 않은 읽기)
* READ COMMITTED(커밋된 읽기)
* REPEATABLE READ(반복 가능한 읽기)
* SERIALIZABLE(직렬화 가능)


* check tx_isolation
```
MariaDB [test]> SHOW VARIABLES LIKE 'tx_isolation';
+---------------+-----------------+
| Variable_name | Value           |
+---------------+-----------------+
| tx_isolation  | REPEATABLE-READ |
+---------------+-----------------+
set tx_isolation = 'READ-UNCOMMITTED'

SELECT @@GLOBAL.tx_isolation, @@tx_isolation, @@session.tx_isolation;
```


* transaction lock timeout
```
MariaDB [test]> select @@innodb_lock_wait_timeout;
+----------------------------+
| @@innodb_lock_wait_timeout |
+----------------------------+
|                         50 |
+----------------------------+

SET GLOBAL innodb_lock_wait_timeout = 5;
```
