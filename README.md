# zuiyou
This project is built for matching the requirement of an interview. 
Further info can be found at
http://www.ixiaochuan.cn/school/2017/index.html  #Q1 #Log Analysis

I develop it on Mac OS. It is theoretically executable on Linux but not expected.

I have simply written a Makefile to build this project.

You will find an executable file named main in the same directory after typing "make".

Have it executed by typing:

./main ~/Desktop/server_access.log

Then it will be processing data in ~/Desktop/server_access.log

Update 2017/4/24.2

补充一下空间效率的分析，整体来看空间只取决于接口的数目，和起的线程数，与log的条目数无关。本题有20个接口，无论是100000条数据，还是200000条数据，空间都是一样的。

另外不是线程起的越多越好，在数据不太大的情况下，太多线程反而影响效率。

Update 2017/4/24

之前说的bug解决。读取文档的时候如果使用feof判断是否读完，只有当#读到文档末尾并再次发生读写操作的时候#才会返回EOF，
导致最后一行读两遍。为了解决这个问题需要在读完文档后额外加一步判定。

Update 2017/4/23

先说结果，运行时间在1-2s，会输出一个output.txt的文件，输出格式为：

接口名 访问次数 平均时间 响应时间超过0.1s的次数

并且按照访问次数降序排列。

遗留一个bug，log文件的最后一行会被读两遍，很奇怪，不知怎么解决。这使得出现在最后一行的那个接口，会多统计一次。

基本思想是起多个线程，程序的主线负责从log文件中读raw data，然后分发给各分线程。

分线程统计处理好后填入一个统一的全局变量。简单构图如下：

   
    ——————> thread_1 ——————> 
    
log ——————> thread_2 ——————> Result List

    ——————> thread_n ——————>
    
算是一个简单的mapreduce过程吧。

log到各thread通信用的是循环缓冲区，不加锁，通过读索引和写索引进行同步

各thread汇总到最终的Result list时，采用了hash算法，每个接口都对应一个list上的索引。

最终，把result list里的元素取出，排序，就是最终结果了。

虽然log文件不小，但最终只有20个接口。

时间复杂度

主线上是O(n)，会把整个log从头到尾读一遍

分线程汇总到Result list的时候用了hash，效率是O(1)。

最后有一步排序，我排序的方法是构造一个链表，hash里的节点有序插入。也就是插入排序。最差效率为O(n^2)。

空间复杂度

处理raw data的空间是m✖️n，m代表队列数，n是队列长度。我现在的代码里起了8个队列，每队128，所以同时会有8✖️128行的数据存入内存；

另外还有Result list，就取决于hash表长度了。最好的是hash长度刚好为接口数。但是不可能，那样冲突太多。我使用了一个719长度的组来代表hashtable。

另外在最终排序的时候申请了额外空间，是n，也就是接口数量。

