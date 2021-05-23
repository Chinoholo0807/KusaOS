# KusaOS
- A mini-OS based on Orange’s
- How to use it:
  run the command 'sudo ./run.sh'

  
  ![image](https://user-images.githubusercontent.com/54203997/119246191-14a71880-bbb2-11eb-9d9c-4c34e1726cdd.png)
  
  
  ![image](https://user-images.githubusercontent.com/54203997/119246194-18d33600-bbb2-11eb-8aed-e0bb5783b270.png)

### Kusa-OS操作系统的编译和运行
如果您想要编译运行Kusa-OS操作系统，需要有如下配置：
1.	Ubuntu 14.04.6
2. 	GCC 4.8.4
3.	NASM 2.10.09
4.	GNU Maker 3.81
5.	Bochs x86 Emulator 2.6.10
此外，您还需要对bochsrc文件进行修改，将相关路径换成您计算机中的路径，完成这些工作之后以超级用户权限运行文件夹中的run.sh即可
 
  
### Kusa-OS操作系统的使用
成功开启Kusa-OS操作系统后，会打印磁盘的相关信息，接下来会进行shell命令的解压缩过程，完成后会播放Kusa-OS的开机动画，动画播放完毕后，屏幕底部会显示shell输入提示，下面是shell命令的介绍：
注：以下pathname均不支持以”.”表示当前路径和以“..”表示父路径
- man：打印指令手册Manu，会显示当前可以使用的指令
-	echo ： 回显输入的参数
-	fs：
    - fs ls：打印当前工作目录下的文件列表
    - fs touch <pathname>：创建一个相对（绝对）路径为pathname的文件
    - fs mkdir <pathname>：创建一个相对（绝对）路径为pathname的目录
    - fs rm <pathname>：删除一个相对（绝对）路径为pathname的文件或目录fs rename <pathname> <name>：将一个相对（绝对）路径为pathname的文件或目录改名为name
- pwd：打印当前工作路径
-	cd [pathname]：不含参数时返回根目录，不然进入pathname目录
-	ps：负责打印当前的进程状态，包括进程的类型、名字、pid、父进程pid、优先级
-	kill <pid>:杀死指定pid的进程
-	csche [nr_policy]：不带参数时显示提示信息和当前系统采用的调度策略，带参数时将当前调度策略调整到nr_policy,然后打印当前系统采用的调度策略
-	testproc：
    - testproc normal：创建一个进程，该进程每个一段时间打印1次，打印10次后结束
    - testproc loop：创建一个进程，该进程会一直执行，每隔一段时间打印1次
    - testproc sched：分别创建3个进程A、B、C，每个进程每隔一段时间打印1次，其中进程A优先级为2，进程B优先级为3，进程C优先级为4

-	mstat：查看虚拟机能使用的内存总大小与当前进程被分配的基地址。
-	testmm：创建一个进程，递归调用自身fork子进程，从当前pid至产生pid=20的子进程为止，后者诞生后所有进程结束。以此来测试进程调度、内存分配与监视等功能
-	buddy：与内存管理模块通信，选择使用buddy算法进行内存分配测试。
-	monitor：创建一个进程，监视并打印进程产生、释放相关的Pid、内存分配情况。
  
#### 其他操作：
  
-	使用PgUp、PgDn切换控制台
-	使用Shift+↑、Shift+↓来滚动屏幕
