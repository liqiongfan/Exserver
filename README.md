## Exserver ###

### Exserver系统架构 ###

![](./Exserver.png)



## Apache ab 命令压测

连接keep-alive情况下，由于Apache ab命令默认情况下是使用HTTP/1.0协议，因此默认情况下，每次请求都进行关闭，如果需要使用连接复用，需要添加 -k参数

### 压测的环境与开启的进程数量

**OS**: Linux MacBookPro-Josin 4.15.0-29debian-generic #31 SMP Fri Jul 27 07:12:08 UTC 2018 x86_64 GNU/Linux

进程开启 **3** 个，外加 **1** 个主进程：

```bash
josin@MacBookPro-Josin:~/Desktop/sockets/build$ ps aux | grep sockets
josin    23779  0.0  0.0   4172   672 pts/0    S+   08:51   0:00 ./sockets
josin    23780  0.5  0.0   4304    76 pts/0    S+   08:51   0:02 ./sockets
josin    23781  0.5  0.0   4304    76 pts/0    S+   08:51   0:02 ./sockets
josin    23782  0.5  0.0   4304    76 pts/0    S+   08:51   0:02 ./sockets
```

### 连接复用 -k 压测结果

```bash
josin@MacBookPro-Josin:~$ ab -k -n1000000 -c1000 http://localhost:8181/
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 100000 requests
Completed 200000 requests
Completed 300000 requests
Completed 400000 requests
Completed 500000 requests
Completed 600000 requests
Completed 700000 requests
Completed 800000 requests
Completed 900000 requests
Completed 1000000 requests
Finished 1000000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            8181

Document Path:          /
Document Length:        5 bytes

Concurrency Level:      1000
Time taken for tests:   7.169 seconds
Complete requests:      1000000
Failed requests:        0
Keep-Alive requests:    1000000
Total transferred:      92000000 bytes
HTML transferred:       5000000 bytes
Requests per second:    139493.11 [#/sec] (mean)
Time per request:       7.169 [ms] (mean)
Time per request:       0.007 [ms] (mean, across all concurrent requests)
Transfer rate:          12532.58 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.4      0      18
Processing:     2    7   0.7      7      18
Waiting:        0    7   0.7      7      18
Total:          2    7   0.8      7      27

Percentage of the requests served within a certain time (ms)
  50%      7
  66%      7
  75%      7
  80%      7
  90%      7
  95%      7
  98%      9
  99%      9
 100%     27 (longest request)
```

连接复用的情况下，系统的性能呈现极佳的表现，能达到每秒 13.9w 的处理能力



### 连接不复用的压测

```bash
josin@MacBookPro-Josin:~$ ab -n1000000 -c1000 http://localhost:8181/
This is ApacheBench, Version 2.3 <$Revision: 1826891 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking localhost (be patient)
Completed 100000 requests
Completed 200000 requests
Completed 300000 requests
Completed 400000 requests
Completed 500000 requests
Completed 600000 requests
Completed 700000 requests
Completed 800000 requests
Completed 900000 requests
Completed 1000000 requests
Finished 1000000 requests


Server Software:        
Server Hostname:        localhost
Server Port:            8181

Document Path:          /
Document Length:        5 bytes

Concurrency Level:      1000
Time taken for tests:   41.183 seconds
Complete requests:      1000000
Failed requests:        0
Total transferred:      87000000 bytes
HTML transferred:       5000000 bytes
Requests per second:    24281.86 [#/sec] (mean)
Time per request:       41.183 [ms] (mean)
Time per request:       0.041 [ms] (mean, across all concurrent requests)
Transfer rate:          2063.01 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   19  24.1     18    1056
Processing:     2   22   5.7     22      82
Waiting:        1   16   5.8     15      79
Total:          4   41  24.3     40    1081

Percentage of the requests served within a certain time (ms)
  50%     40
  66%     42
  75%     43
  80%     43
  90%     45
  95%     48
  98%     52
  99%     56
 100%   1081 (longest request)	
```

不复用连接的情况下，由于增加了多个进程的调度与每个进程的epoll开销，因此性能降低很多，只能达到每秒 2.4w的处理能力

目前暂未添加 **CGI** ，服务器的配置等，感兴趣的可以参与









