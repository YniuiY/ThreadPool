# 项目特点
1. 用C++实现了线程池的功能；
2. 动态线程池：池内线程数可以自适应增减；
3. 本地队列：线程本地队列优化了锁竞争；
4. 任务窃取：核心线程在本地和池任务队列为空时，从相邻核心线程的任务队列获取任务。减少线程空阻塞等待任务的时间，有利于负载均衡；
5. 扇出：线程从任务队列批量获取任务，减少从任务队列获取任务导致锁竞争的次数。


## 线程池的基本思路：
1. 两个队列（容器），一个线程队列，一个任务队列，
2. 从任务队列中取出任务，交给线程队列中的线程执行。
3. 循环往复就是线程池。

## 改进思路：减少锁竞争，增加扇入扇出，优化负载均衡
1. 封装包含本地队列的thread类，实现local queue减少锁竞争。此类的对象从自己独立的任务队列中获取任务，避免和其他线程的锁竞争。
2. 加入扇入扇出的功能，线程从任务队列批量获取任务，减少从任务队列中获取任务的次数。
3. 加入任务窃取功能，主线程在本地任务队列为空，线程池任务队列也为空时主动从其他线程的本地任务队列中获取任务。

### 下一步加入绑定CPU核心的功能
1. 定义绑定接口
2. 针对接口开发功能
3. 测试功能
