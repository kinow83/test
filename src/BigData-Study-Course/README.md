Hadoop Course


ref: hadoop safe mode off (by http://knight76.tistory.com/entry/Hadoop-Name-node-is-in-safe-mode-%EC%97%90%EB%9F%AC-%ED%95%B4%EA%B2%B0)
```sh
Hadoop이 정상적인 종료를 하지 않았을 때, 에러가 난다.
비정상적인 종료시 hadoop 은 safe 모드로 이동하는데. 종료시 아래와 같은 명령을 내려서 restart할 때 문제가 없도록 해야 한다.
$ ./bin/hadoop dfsadmin -safemode leave 
Safe mode is OFF
```


ref: cassandra
```sh
[카산드라 설치]
http://jidasung.blogspot.kr/2012/08/hbase-cassandra.html
[카산드라 사용법]
CQL:  http://jhproject.tistory.com/51

CQL 사용: http://opentutorials.org/module/1074/7483
```
