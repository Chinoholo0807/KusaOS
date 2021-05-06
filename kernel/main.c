
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                     OS Team for CYTUZ, 2020
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task* p_task;
	struct proc* p_proc= proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	u16   selector_ldt = SELECTOR_LDT_FIRST;
        u8    privilege;
        u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS+NR_PROCS; i++) {
	        if (i < NR_TASKS) {     /* 任务 */
                        p_task    = task_table + i;
                        privilege = PRIVILEGE_TASK;
                        rpl       = RPL_TASK;
                        eflags    = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1 */
			prio      = 15;
                }
                else {                  /* 用户进程 */
                        p_task    = user_proc_table + (i - NR_TASKS);
                        privilege = PRIVILEGE_USER;
                        rpl       = RPL_USER;
                        eflags    = 0x202; /* IF=1, bit 2 is always 1 */
			prio      = 5;
                }

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid = i;			/* pid */

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty		= 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;
		p_proc->run_state=1;
		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

        /* proc_table[NR_TASKS + 0].nr_tty = 0; */
        /* proc_table[NR_TASKS + 1].nr_tty = 1; */
        /* proc_table[NR_TASKS + 2].nr_tty = 1; */

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
        kbInitial();

	restart();

	while(1){}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}
/*****************************************************************************
 *                                fs_support
 *****************************************************************************/
PUBLIC void addTwoString(char *to_str,char *from_str1,char *from_str2){
    int i=0,j=0;
    while(from_str1[i]!=0)
        to_str[j++]=from_str1[i++];
    i=0;
    while(from_str2[i]!=0)
        to_str[j++]=from_str2[i++];
    to_str[j]=0;
}
PUBLIC void convert_to_absolute(char* dest, char* path, char* file)
{
    int i=0, j=0;
    while (path[i] != 0)  // 写入路径
    {
        dest[j] = path[i];
        j++;
        i++;
    }
    i = 0;
    while (file[i] != 0)  // 写入文件名
    {
        dest[j] = file[i];
        j++;
        i++;
    }
    dest[j] = 0;  // 结束符
}
void CreateFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);

    int fd = open(absoPath, O_CREAT | O_RDWR);

    if (fd == -1)
    {
        printf("Failed to create a new file with name %s\n", file);
        return;
    }

    char buf[1] = {0};
    write(fd, buf, 1);
    printf("File created: %s (fd %d)\n", file, fd);
    close(fd);
}

void DeleteFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int m=unlink(absoPath);
    if (m == 0)
        printf("%s deleted!\n", file);
    else
        printf("Failed to delete %s!\n", file);
}

void ReadFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
    {
        printf("Failed to open %s!\n", file);
        return;
    }

    char buf[4096];
    int n = read(fd, buf, 4096);
    if (n == -1)  // 读取文件内容失败
    {
        printf("An error has occured in reading the file!\n");
        close(fd);
        return;
    }

    printf("%s\n", buf);
    close(fd);
}

void WriteFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
    {
        printf("Failed to open %s!\n", file);
        return;
    }

    char tty_name[] = "/dev_tty0";
    int fd_stdin  = open(tty_name, O_RDWR);
    if (fd_stdin == -1)
    {
        printf("An error has occured in writing the file!\n");
        return;
    }
    char writeBuf[4096];  // 写缓冲区
    int endPos = read(fd_stdin, writeBuf, 4096);
    writeBuf[endPos] = 0;
    write(fd, writeBuf, endPos + 1);  // 结束符也应写入
    close(fd);
}

void CreateDir(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);

    if (fd != -1)
    {
        printf("Failed to create a new directory with name %s\n", file);
        return;
    }
    mkdir(absoPath);
}

void GoDir(char* path, char* file)
{
    int flag = 0;  // 判断是进入下一级目录还是返回上一级目录
    char newPath[512] = {0};
    if (file[0] == '.' && file[1] == '.')  // cd ..返回上一级目录
    {
        flag = 1;
        int pos_path = 0;
        int pos_new = 0;
        int i = 0;
        char temp[128] = {0};  // 用于存放某一级目录的名称
        while (path[pos_path] != 0)
        {
            if (path[pos_path] == '/')
            {
                pos_path++;
                if (path[pos_path] == 0)  // 已到达结尾
                    break;
                else
                {
                    temp[i] = '/';
                    temp[i + 1] = 0;
                    i = 0;
                    while (temp[i] != 0)
                    {
                        newPath[pos_new] = temp[i];
                        temp[i] = 0;  // 抹掉
                        pos_new++;
                        i++;
                    }
                    i = 0;
                }
            }
            else
            {
                temp[i] = path[pos_path];
                i++;
                pos_path++;
            }
        }
    }
    char absoPath[512];
    char temp[512];
    int pos = 0;
    while (file[pos] != 0)
    {
        temp[pos] = file[pos];
        pos++;
    }
    temp[pos] = '/';
    temp[pos + 1] = 0;
    if (flag == 1)  // 返回上一级目录
    {
        temp[0] = 0;
        convert_to_absolute(absoPath, newPath, temp);
    }
    else  // 进入下一级目录
        convert_to_absolute(absoPath, path, temp);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
        printf("%s is not a directory!\n", absoPath);
    else
        memcpy(path, absoPath, 512);
}

/*======================================================================*
                            welcome animation
*======================================================================*/
void hello_cytuz()
{
clear();
printf("                                                                        \n");
milli_delay(2000);
printf("                                                                        \n");
milli_delay(2000);
printf("          ********   **    ***   *********    ***        ***   ******** \n");
milli_delay(2000);
printf("        /**          /**  /**    //*******   /**        /***     /****  \n");
milli_delay(2000);
printf("        /**           /**/**       /***      /**        /***    /***    \n");
milli_delay(200);
printf("       /**             /**         /***      /**        /***   /**      \n");
milli_delay(200);
printf("       /**             /**         /***      //**      /***   /**       \n");
milli_delay(2000);
printf("       //**            /**         /***      //***********   /****      \n");
milli_delay(200);
printf("        //*******      /**         /***       //*********   /********   \n");
milli_delay(200);
printf("        ///////        ///         ///         /////////   /////////    \n");
milli_delay(20000);
printf("                                                                        \n");
milli_delay(2000);
printf("                                                                        \n");
milli_delay(2000);
printf("                              contributors                              \n");
milli_delay(2000);
printf("========================================================================\n");
milli_delay(2000);
printf("      1851910      1852140      1852141      1852715      1853201       \n");
milli_delay(2000);
printf("========================================================================\n");
milli_delay(20000);
}

void displayWelcomeInfo() 
{
    printf("   |=========================================================================|\n");
    printf("   |=========================================================================|\n");
    printf("   |               **       * *       ***       * *       ***                |\n");
    printf("   |              *          *         *        * *        *                 |\n");
    printf("   |               **        *         *        ***       ***                |\n");
    printf("   |=========================================================================|\n");
    printf("   |                                                                         |\n");
    printf("   |                        OPERATING        SYSTEM                          |\n");
    printf("   |                                                                         |\n");
    printf("   |                        INITIALIZATION COMPLETE                          |\n");
    printf("   |                                                                         |\n");
    printf("   |                           Welcome to CYTUZ                              |\n");
    printf("   |                      Enter 'help' to show commands.                     |\n");
    printf("   |=========================================================================|\n");
    printf("   |=========================================================================|\n");
    printf("\n\n\n\n");


}


/*****************************************************************************
 *                                Custom Command
 *****************************************************************************/
char* findpass(char *src)
{
    char pass[128];
    int flag = 0;
    char *p1, *p2;

    p1 = src;
    p2 = pass;

    while (p1 && *p1 != ' ')
    {
        if (*p1 == ':')
            flag = 1;

        if (flag && *p1 != ':')
        {
            *p2 = *p1;
            p2++;
        }
        p1++;
    }
    *p2 = '\0';

    return pass;
}

void clearArr(char *arr, int length)
{
    int i;
    for (i = 0; i < length; i++)
        arr[i] = 0;
}

void printTitle()
{
    clear(); 	
    if(current_console==0){
    	displayWelcomeInfo();
    }
    else{
    	printf("[TTY #%d]\n", current_console);
    }
    
}

void clear()
{	
	clear_screen(0,console_table[current_console].cursor);
    console_table[current_console].crtc_start = console_table[current_console].orig;
    console_table[current_console].cursor = console_table[current_console].orig;    
}

void help()
{
    printf("   |=========================================================================|\n");
    printf("   |======================= C     Y     T     U     Z =======================|\n");
    printf("   |=========================================================================|\n");
    printf("   |                                                                         |\n");
    printf("   |************************* COMMANDS FOR SYSTEM ***************************|\n");
    printf("   |                                                                         |\n");
    printf("   |                               help : Show CYTUZ commands                |\n");
    printf("   |                              clear : Clear the screen                   |\n");
    printf("   |                            process : Show process manager               |\n");
    printf("   |************************* COMMANDS FOR FILE SYSTEM **********************|\n");
    printf("   |                                 ls : List files in current directory    |\n");
    printf("   |                              touch : Create a new file                  |\n");
    printf("   |                                cat : View a file                        |\n");
    printf("   |                                 cp : Copy a file                        |\n");
    printf("   |                                 mv : Move a file                        |\n");   
    printf("   |                                 dm : Encrypt a file                     |\n");
    printf("   |                                 vi : Modify a file                      |\n");
    printf("   |                                 rm : Delete a file                      |\n");
    printf("   |                                 cd : Change the directory               |\n");
    printf("   |                              mkdir : Create a new directory             |\n");
    printf("   |                                                                         |\n");
    printf("   |************************* COMMANDS FOR USER APPLICATIONS ****************|\n");
    printf("   |                        minesweeper : Launch Minesweeper                 |\n");
    printf("   |                           game2048 : Launch 2048Game                    |\n");
    printf("   |                              guess : Launch GuessNumberGame             |\n");
    printf("   |                             gobang : Launch GoBangGame                  |\n");
    printf("   |                                toe : Launch Tic-Tac-Toe Game            |\n");
    printf("   |                              queen : Launch Strange N Queen Game        |\n");
    printf("   |                         calculator : Launch calculator                  |\n");
    printf("   |=========================================================================|\n");
}

void ProcessManage()
{
    int i;
    printf("=============================================================================\n");
    printf("      processID      |    name       | spriority    | running?\n");
    //进程号，进程名，优先级，是否是系统进程，是否在运行
    printf("-----------------------------------------------------------------------------\n");
    for ( i = 0 ; i < NR_TASKS + NR_PROCS ; ++i )//逐个遍历
    {
        /*if ( proc_table[i].priority == 0) continue;//系统资源跳过*/
		if(proc_table[i].p_flags!=1)
        printf("        %d           %s            %d                %d\n", proc_table[i].pid, proc_table[i].name, 		
											proc_table[i].priority,proc_table[i].run_state);}
    printf("=============================================================================\n");
	printf("=                 pause  [pid]  you can pause one process                   =\n");
	printf("=          	      resume [pid]  you can resume one process                  =\n");
	printf("=                 kill   [pid]  kill the process                            =\n");
	printf("=                 up     [pid]  improve the process priority                =\n");
	printf("=============================================================================\n");
}

//游戏运行库
#define chartonumber(x) (x-'0')
unsigned int _seed2 = 0xDEADBEEF;

void srand(unsigned int seed){
	_seed2 = seed;
}

int rand() {
	int next = _seed2;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (next / 65536) ;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (next / 65536) ;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (next / 65536) ;

	_seed2 = next;

	return result>0 ? result : -result;
}
/*****************************************************************************
 *                                game code for Minesweeper
 *****************************************************************************/
#define rows 11
#define cols 11
#define Count 10

char mine[rows][cols];
char show[rows][cols];

void m_init()
{
	int i = 0;
	int j = 0;
	for (i = 0; i < rows - 1; i++)
	{
		for (j = 0; j < cols - 1; j++)
		{
			mine[i][j] = '0';
			show[i][j] = '*';
		}
	}
}

void m_set_mine()
{
    int x = 0;
    int y = 0;
    int count = 10;//雷总数
    while (count)//雷布完后跳出循环
    {
        int x = rand() % 10 + 1;//产生1到10的随机数，在数组下标为1到10的范围内布雷
        int y = rand() % 10 + 1;//产生1到10的随机数，在数组下标为1到10的范围内布雷
        if (mine[x][y] == '0')//找不是雷的地方布雷
        {
            mine[x][y] = '1';
            count--;
        }
    }
}


void m_display(char a[rows][cols])
{
	clear();
	printf("   |=========================================================================|\n");
	printf("   |======================= Minesweeper for CYTUZ ===========================|\n");
	printf("   |=========================================================================|\n");
	printf("   |                 A long time ago in a galaxy far,far away...             |\n");
	printf("   | YOU need to sweep Imperial minefield to deliver Death Star blueprints!  |\n");
	printf("   |                   The fate of galaxy is at your hands!                  |\n");
	printf("   |                           Enter q to quit                               |\n");
	printf("   |=========================================================================|\n");
	printf("   |=========================================================================|\n");
	int i = 0;
	int j = 0;
	printf("\n");
	printf(" ");
	for (i = 1; i < cols - 1; i++)
	{
		printf(" %d ", i);
	}
	printf("\n");
	for (i = 1; i < rows - 1; i++)
	{
		printf("%d", i);
		for (j = 1; j < cols - 1; j++)
		{
			printf(" %c ", a[i][j]);
		}
		printf("\n");
	}
}

  
int m_get_num(int x, int y)
{
	int count = 0;
	if (mine[x - 1][y - 1] == '1')
	{
		count++;
	}
	if (mine[x - 1][y] == '1')
	{
		count++;
	}
	if (mine[x - 1][y + 1] == '1')
	{
		count++;
	}
	if (mine[x][y - 1] == '1')  
	{
		count++;
	}
	if (mine[x][y + 1] == '1')  
	{
		count++;
	}
	if (mine[x + 1][y - 1] == '1')  
	{
		count++;
	}
	if (mine[x + 1][y] == '1')  
	{
		count++;
	}
	if (mine[x + 1][y + 1] == '1')  
	{
		count++;
	}
	return  count;
}
  
int m_sweep()
{
	int count = 0;
	int x = 0, y = 0;
	char cx[2], cy[2];
	while (count != ((rows - 2)*(cols - 2) - Count))
	{
		printf("Please input row number: ");
		int r = read(0, cx, 2);
		if (cx[0] == 'q')
			return 0;
		x = cx[0] - '0';
		while (x <= 0 || x > 9)
		{
			printf("Wrong Input!\n");
			printf("Please input row number: ");
			r = read(0, cx, 2);
			if (cx[0] == 'q')
				return 0;
			x = cx[0] - '0';
		}

		printf("Please input col number: ");
		r = read(0, cy, 2);
		if (cy[0] == 'q')
			return 0;
		y = cy[0] - '0';
		while (y <= 0 || y > 9)
		{
			printf("Wrong Input!\n");
			printf("Please input col number: ");
			r = read(0, cy, 2);
			if (cy[0] == 'q')
				return 0;
			y = cy[0] - '0';
		}

		if (mine[x][y] == '1')
		{
			m_display(mine);
			printf("Nerf this! Game Over!\n");

			return 0;
		}
		else
		{
			int ret = m_get_num(x, y);
			show[x][y] = ret + '0';
			m_display(show);
			count++;
		}
	}
	printf("YOU WIN!\n");
	m_display(mine);
	return 0;
}

int runMine(fd_stdin, fd_stdout)
{
	printf("   |=========================================================================|\n");
	printf("   |======================= Minesweeper for CYTUZ ===========================|\n");
	printf("   |=========================================================================|\n");
	printf("   |                 A long time ago in a galaxy far,far away...             |\n");
	printf("   | YOU need to sweep Imperial minefield to deliver Death Star blueprints!  |\n");
	printf("   |                   The fate of galaxy is at your hands!                  |\n");
	printf("   |                           Enter q to quit                               |\n");
	printf("   |=========================================================================|\n");
	printf("   |=========================================================================|\n");
        
	
	

	m_init();
	m_set_mine();
	m_display(show);
	m_sweep();

	printf("\nEnter anything to continue...");
	char rdbuf[128];
	int r = read(fd_stdin, rdbuf, 70);
	rdbuf[r] = 0;
	while (r < 1)
	{
		r = read(fd_stdin, rdbuf, 70);
	}
	clear();
	printf("\n");
	return 0;
}
/*****************************************************************************
 *                           code for game 2048
 *****************************************************************************/
#define KEY_CODE_UP    0x41
#define KEY_CODE_DOWN  0x42
#define KEY_CODE_LEFT  0x44
#define KEY_CODE_RIGHT 0x43
#define KEY_CODE_QUIT  0x71




static char config_path[4096] = { 0 }; /* 配置文件路径 */

void init_game2048();    /* 初始化游戏 */
static void loop_game(int fd_stdin);    /* 游戏循环 */
static void reset_game();   /* 重置游戏 */
static void release_game(int signal); /* 释放游戏 */

static char* read_keyboard(int fd_stdin);

static void move_left();  /* 左移 */
static void move_right(); /* 右移 */
static void move_up();    /* 上移 */
static void move_down();  /* 下移 */

static void add_rand_num();    /* 生成随机数，本程序中仅生成2或4，概率之比设为9:1 */
static void check_game_over(); /* 检测是否输掉游戏，设定游戏结束标志 */
static int get_null_count();   /* 获取游戏面板上空位置数量 */
static void refresh_show();    /* 刷新界面显示 */

static int board[4][4];     /* 游戏数字面板，抽象为二维数组 */
static int score;           /* 游戏得分 */
static int best;            /* 游戏最高分 */
static int if_need_add_num; /* 是否需要生成随机数标志，1表示需要，0表示不需要 */
static int if_game_over;    /* 是否游戏结束标志，1表示游戏结束，0表示正常 */
static int if_prepare_exit; /* 是否准备退出游戏，1表示是，0表示否 */

/* main函数 函数定义 */
void Run2048(fd_stdin, fd_stdout)
{
	clear();
	init_game2048();
	loop_game(fd_stdin);
	release_game(0);
}

/* 开始游戏 函数定义 */
void loop_game(int fd_stdin) {
	while (1) {
		/* 接收标准输入流字符命令 */
		char rdbuf[128];
		int r = 0;
		r = read(fd_stdin, rdbuf, 70);
		if (r > 1)
		{
			refresh_show();
			continue;
		}
		rdbuf[r] = 0;
		char cmd = rdbuf[0];
		/* 判断是否准备退出游戏 */
		if (if_prepare_exit) {
			if (cmd == 'y' || cmd == 'Y') {
				/* 退出游戏，清屏后退出 */
				clear();
				return;
			}
			else if (cmd == 'n' || cmd == 'N') {
				/* 取消退出 */
				if_prepare_exit = 0;
				refresh_show();
				continue;
			}
			else {
				continue;
			}
		}

		/* 判断是否已经输掉游戏 */
		if (if_game_over) {
			if (cmd == 'y' || cmd == 'Y') {
				/* 重玩游戏 */
				reset_game();
				continue;
			}
			else if (cmd == 'n' || cmd == 'N') {
				/* 退出游戏，清屏后退出  */
				clear();
				return;
			}
			else {
				continue;
			}
		}

		if_need_add_num = 0; /* 先设定不默认需要生成随机数，需要时再设定为1 */
		/* 命令解析，上下左右箭头代表上下左右命令，q代表退出 */
		switch (cmd) {
		case 'a':
			move_left();
			break;
		case 's':
			move_down();
			break;
		case 'w':
			move_up();
			break;
		case 'd':
			move_right();
			break;
		case 'q':
			if_prepare_exit = 1;
			break;
		default:
			refresh_show();
			continue;
		}

		/* 默认为需要生成随机数时也同时需要刷新显示，反之亦然 */
		if (if_need_add_num) {
			add_rand_num();
			refresh_show();
		}
		else if (if_prepare_exit) {
			refresh_show();
		}
	}
}

/* 重置游戏 函数定义 */
void reset_game() {
	score = 0;
	if_need_add_num = 1;
	if_game_over = 0;
	if_prepare_exit = 0;

	/* 了解到游戏初始化时出现的两个数一定会有个2，所以先随机生成一个2，其他均为0 */
	int n = get_ticks() % 16;
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			board[i][j] = (n-- == 0 ? 2 : 0);
		}
	}

	/* 前面已经生成了一个2，这里再生成一个随机的2或者4，概率之比9:1 */
	add_rand_num();

	/* 在这里刷新界面并显示的时候，界面上已经默认出现了两个数字，其他的都为空（值为0） */
	refresh_show();
}

/* 生成随机数 函数定义 */
void add_rand_num() {
	int n = get_ticks() % get_null_count(); /* 确定在何处空位置生成随机数 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			/* 定位待生成的位置 */
			if (board[i][j] == 0 && n-- == 0) {
				board[i][j] = (get_ticks() % 10 ? 2 : 4); /* 生成数字2或4，生成概率为9:1 */
				return;
			}
		}
	}
}

/* 获取空位置数量 函数定义 */
int get_null_count() {
	int n = 0;
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 4; ++j) {
			board[i][j] == 0 ? ++n : 1;
		}
	}
	return n;
}

/* 检查游戏是否结束 函数定义 */
void check_game_over() {
	int i;
	for (i = 0; i < 4; ++i) {
		int j;
		for (j = 0; j < 3; ++j) {
			/* 横向和纵向比较挨着的两个元素是否相等，若有相等则游戏不结束 */
			if (board[i][j] == board[i][j + 1] || board[j][i] == board[j + 1][i]) {
				if_game_over = 0;
				return;
			}
		}
	}
	if_game_over = 1;
}

/*
 * 如下四个函数，实现上下左右移动时数字面板的变化算法
 * 左和右移动的本质一样，区别仅仅是列项的遍历方向相反
 * 上和下移动的本质一样，区别仅仅是行项的遍历方向相反
 * 左和上移动的本质也一样，区别仅仅是遍历时行和列互换
*/

/*  左移 函数定义 */
void move_left() {
	/* 变量i用来遍历行项的下标，并且在移动时所有行相互独立，互不影响 */
	int i;
	for (i = 0; i < 4; ++i) {
		/* 变量j为列下标，变量k为待比较（合并）项的下标，循环进入时k<j */
		int j, k;
		for (j = 1, k = 0; j < 4; ++j) {
			if (board[i][j] > 0) /* 找出k后面第一个不为空的项，下标为j，之后分三种情况 */
			{
				if (board[i][k] == board[i][j]) {
					/* 情况1：k项和j项相等，此时合并方块并计分 */
					score += board[i][k++] *= 2;
					board[i][j] = 0;
					if_need_add_num = 1; /* 需要生成随机数和刷新界面 */
				}
				else if (board[i][k] == 0) {
					/* 情况2：k项为空，则把j项赋值给k项，相当于j方块移动到k方块 */
					board[i][k] = board[i][j];
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else {
					/* 情况3：k项不为空，且和j项不相等，此时把j项赋值给k+1项，相当于移动到k+1的位置 */
					board[i][++k] = board[i][j];
					if (j != k) {
						/* 判断j项和k项是否原先就挨在一起，若不是则把j项赋值为空（值为0） */
						board[i][j] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 右移 函数定义 */
void move_right() {
	/* 仿照左移操作，区别仅仅是j和k都反向遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 2, k = 3; j >= 0; --j) {
			if (board[i][j] > 0) {
				if (board[i][k] == board[i][j]) {
					score += board[i][k--] *= 2;
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else if (board[i][k] == 0) {
					board[i][k] = board[i][j];
					board[i][j] = 0;
					if_need_add_num = 1;
				}
				else {
					board[i][--k] = board[i][j];
					if (j != k) {
						board[i][j] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 上移 函数定义 */
void move_up() {
	/* 仿照左移操作，区别仅仅是行列互换后遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 1, k = 0; j < 4; ++j) {
			if (board[j][i] > 0) {
				if (board[k][i] == board[j][i]) {
					score += board[k++][i] *= 2;
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else if (board[k][i] == 0) {
					board[k][i] = board[j][i];
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else {
					board[++k][i] = board[j][i];
					if (j != k) {
						board[j][i] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 下移 函数定义 */
void move_down() {
	/* 仿照左移操作，区别仅仅是行列互换后遍历，且j和k都反向遍历 */
	int i;
	for (i = 0; i < 4; ++i) {
		int j, k;
		for (j = 2, k = 3; j >= 0; --j) {
			if (board[j][i] > 0) {
				if (board[k][i] == board[j][i]) {
					score += board[k--][i] *= 2;
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else if (board[k][i] == 0) {
					board[k][i] = board[j][i];
					board[j][i] = 0;
					if_need_add_num = 1;
				}
				else {
					board[--k][i] = board[j][i];
					if (j != k) {
						board[j][i] = 0;
						if_need_add_num = 1;
					}
				}
			}
		}
	}
}

/* 刷新界面 函数定义 */
void refresh_show() {
	clear();

	printf("\n\n\n\n");
	printf("                  GAME: 2048     SCORE: %05d    \n", score);
	printf("               --------------------------------------------------");

	/* 绘制方格和数字 */
	printf("\n\n                             |----|----|----|----|\n");
	int i;
	for (i = 0; i < 4; ++i) {
		printf("                             |");
		int j;
		for (j = 0; j < 4; ++j) {
			if (board[i][j] != 0) {
				if (board[i][j] < 10) {
					printf("  %d |", board[i][j]);
				}
				else if (board[i][j] < 100) {
					printf(" %d |", board[i][j]);
				}
				else if (board[i][j] < 1000) {
					printf(" %d|", board[i][j]);
				}
				else if (board[i][j] < 10000) {
					printf("%4d|", board[i][j]);
				}
				else {
					int n = board[i][j];
					int k;
					for (k = 1; k < 20; ++k) {
						n = n >> 1;
						if (n == 1) {
							printf("2^%02d|", k); /* 超过四位的数字用2的幂形式表示，如2^13形式 */
							break;
						}
					}
				}
			}
			else printf("    |");
		}

		if (i < 3) {
			printf("\n                             |----|----|----|----|\n");
		}
		else {
			printf("\n                             |----|----|----|----|\n");
		}
	}
	printf("\n");
	printf("               --------------------------------------------------\n");
	printf("                  [W]:UP [S]:DOWN [A]:LEFT [D]:RIGHT [Q]:EXIT\n");
	printf("                  Enter your commond here:");

	if (get_null_count() == 0) {
		check_game_over();

		/* 判断是否输掉游戏 */
		if (if_game_over) {
			printf("\r                      \nGAME OVER! TRY THE GAME AGAIN? [Y/N]:     \b\b\b\b");
		}
	}

	/* 判断是否准备退出游戏 */
	if (if_prepare_exit) {
		printf("\r                   \nDO YOU REALLY WANT TO QUIT THE GAME? [Y/N]:   \b\b");

	}
}

/* 初始化游戏 */
void init_game2048() {
	reset_game();
}

/* 释放游戏 */
void release_game(int signal) {
	clear();

	/*if (signal == SIGINT) {
		printf("\n");
	}*/

}

 /*======================================================================*
							Tic-Tac-Toe
 *======================================================================*/


typedef char chess[10];
typedef int temparr[10];
chess arr;
temparr brr;
int number, suc, n3, c3, n2, c2, n1, c1;
char ch;
void inarrdata(chess a)
{
	a[1] = '1'; a[2] = '2'; a[3] = '3';
	a[4] = '4'; a[5] = '5'; a[6] = '6';
	a[7] = '7'; a[8] = '8'; a[9] = '9';
}
void display(chess a)
{
	printf("              \n"); printf("\n");
	printf("               %c | %c | %c\n", a[1], a[2], a[3]);
	printf("               --------------\n");
	printf("               %c | %c | %c\n", a[4], a[5], a[6]);
	printf("               --------------\n");
	printf("               %c | %c | %c\n", a[7], a[8], a[9]);
	printf("              \n"); printf("\n");
}
int arrfull()
{
	int i;
	int arrf = 0;
	for (i = 1; i <= 9; i++)
		if (i == arr[i] - 48)
			arrf = 1;
	return arrf;
}
void cn(int line)
{
	switch (line)
	{
	case 0:c3 = c3 + 1; break;
	case 1:n2 = n2 + 1; break;
	case 2:c2 = c2 + 1; break;
	case 3:n1 = n1 + 1; break;
	case 4:c1 = c1 + 1; break;
	case 5:n3 = n3 + 1; break;
	}
}
int linenum(char a, char b, char c)
{
	int ln = 6;
	if ((a == 'X') && (b == 'X') && (c == 'X'))
		ln = 0;
	if (((a == 'O') && (b == 'O') && (c != 'O')) || ((a == 'O') && (b != 'O') && (c == 'O')) || ((a != 'O') && (b == 'O') && (c == 'O')))
		ln = 1;
	if (((a == 'X') && (b == 'X') && (c != 'X')) || ((a == 'X') && (b != 'X') && (c == 'X')) || ((a != 'X') && (b == 'X') && (c == 'X')))
		ln = 2;
	if (((a == 'O') && (b != 'O') && (c != 'O')) || ((a != 'O') && (b == 'O') && (c != 'O')) || ((a != 'O') && (b != 'O') && (c == 'O')))
		ln = 3;
	if (((a == 'X') && (b != 'X') && (c != 'x')) || ((a != 'X') && (b == 'X') && (c != 'X')) || ((a != 'X') && (b != 'X') && (c == 'X')))
		ln = 4;
	if ((a == 'O') && (b == 'O') && (c == 'O'))
		ln = 5;
	return ln;
}
int maxbrr(int *br)
{
	int temp, i, mb;
	temp = -888;
	for (i = 1; i <= 9; i++)
	{
		if (temp <= br[i])
		{
			temp = br[i];
			mb = i;
		}
	}
	return mb;
}
void manstep() 
{
	int j;
	display(arr);
	if (arrfull()) 
	{
		printf("Which step are you going to take? please enter a number(1--9):");
		char bur[128];
		read(0, bur, 128);
		j = chartonumber(bur[0]);

		arr[j] = 'O';
		c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
		number = linenum(arr[1], arr[2], arr[3]); cn(number);
		number = linenum(arr[4], arr[5], arr[6]); cn(number);
		number = linenum(arr[7], arr[8], arr[9]); cn(number);
		number = linenum(arr[1], arr[4], arr[7]); cn(number);
		number = linenum(arr[2], arr[5], arr[8]); cn(number);
		number = linenum(arr[3], arr[6], arr[9]); cn(number);
		number = linenum(arr[1], arr[5], arr[9]); cn(number);
		number = linenum(arr[3], arr[5], arr[7]); cn(number);
		if (n3 != 0) 
		{
			display(arr);
			printf("\n");
			printf("You win!!!\n");
		
			suc = 0;
		}
	}
}
void computerstep()
{
	int i;
	if (arrfull()) 
	{
		for (i = 1; i <= 9; i++) 
		{
			if (i == arr[i] - 48)
			{
				c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
				arr[i] = 'X';
				number = linenum(arr[1], arr[2], arr[3]); cn(number);
				number = linenum(arr[4], arr[5], arr[6]); cn(number);
				number = linenum(arr[7], arr[8], arr[9]); cn(number);
				number = linenum(arr[1], arr[4], arr[7]); cn(number);
				number = linenum(arr[2], arr[5], arr[8]); cn(number);
				number = linenum(arr[3], arr[6], arr[9]); cn(number);
				number = linenum(arr[1], arr[5], arr[9]); cn(number);
				number = linenum(arr[3], arr[5], arr[7]); cn(number);
				brr[i] = (128 * c3 - 63 * n2 + 31 * c2 - 15 * n1 + 7 * c1); 
				arr[i] = i + 48;
			}
			else
				brr[i] = -999;
		}
		arr[maxbrr(brr)] = 'X'; 
		c3 = 0; n2 = 0; c2 = 0; n1 = 0; c1 = 0;
		number = linenum(arr[1], arr[2], arr[3]); cn(number);
		number = linenum(arr[4], arr[5], arr[6]); cn(number);
		number = linenum(arr[7], arr[8], arr[9]); cn(number);
		number = linenum(arr[1], arr[4], arr[7]); cn(number);
		number = linenum(arr[2], arr[5], arr[8]); cn(number);
		number = linenum(arr[3], arr[6], arr[9]); cn(number);
		number = linenum(arr[1], arr[5], arr[9]); cn(number);
		number = linenum(arr[3], arr[5], arr[7]); cn(number);
		if (c3 != 0) 
		{
			display(arr);
			printf("\n");
			printf("PC win!!!\n");

			suc = 0;
		}
	}
	else
		suc = 0;

}
void toe()
{

	printf("              ***************************************************\n");
	printf("              *               Tic-Tac-Toe                       *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *              Follow the instruction             *\n");
	printf("              *              Enter e to quit                    *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
	inarrdata(arr); 
	display(arr); 
	suc = 1;
	printf("Do you want to go first?(y/n)");
	char bufr[128];
	read(0, bufr, 128);
	if ((bufr[0] == 'y') || (bufr[0] == 'Y')) 
	{
		while (suc)
		{
			manstep();
			computerstep();
		}
		display(arr);
	}
	else 
	{
		while (suc)
		{
			computerstep();
			if (suc)
				manstep();
		}
	}
	if(!arrfull())
		printf("\nNo winer !\n");
}

 /*======================================================================*
						Normal queen
 *======================================================================*/

void huisu(int l);
int jc(int l, int i);
int n, h[100];
int x;

void queen()

{

	printf("              ***************************************************\n");
	printf("              *            Strange N Queen Game                 *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *              First think a num                  *\n");
	printf("              *              Then claculate by yourself         *\n");
	printf("              *              Enter num to certify               *\n");
	printf("              *              Enter e to quit                    *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
	printf("N=");
	char bur[128];
	read(0, bur, 128);
	n = chartonumber(bur[0]);
	x = 0;
	huisu(1);
	printf("There are %d stacking methods\n", x);
}

void huisu(int l)
{
	int i, j;
	if (l == n + 1)
	{
		x = x + 1;
		printf("stacking methods are:\n", x);
		for (i = 1; i <= n; i++)
			printf("%d", h[i]);
		printf("\n");
	}
	for (i = 1; i <= n; i++)
	{
		h[l] = i;
		if (jc(l, i) != 1)
			huisu(l + 1);
	}
}

int jc(int l, int i)
{
	int k;
	for (k = 1; k<l; k++)
		if ((l - k == h[k] - i) || i == h[k])
			return 1;
	return 0;
}

 /*======================================================================*
							calculator
 *======================================================================*/
void calculator()

{
	printf("              ***************************************************\n");
	printf("              *               Calculator                        *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *              Enter simple arithmetic            *\n");
	printf("              *              Enter e to quit                    *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
	while(1){	
	char result;

	char bufr[128];
	read(0, bufr, 128);


	switch(bufr[1])

	{

    	case '+':result=chartonumber(bufr[0])+chartonumber(bufr[2]);break;

	case '-':result=chartonumber(bufr[0])-chartonumber(bufr[2]);break;

	case '*':result=chartonumber(bufr[0])*chartonumber(bufr[2]);break;

	case '/':result=chartonumber(bufr[0])/chartonumber(bufr[2]);break;
	

	}
	if(bufr[0]=='e')
		break;
	else
		printf("%d %c %d = %d\n",chartonumber(bufr[0]),bufr[1],chartonumber(bufr[2]),result);
	}

}
 /*======================================================================*
							guess number
 *======================================================================*/
int my_atoi(const char *s)
{
	int num, i;
	char ch;
	num = 0;
	for (i = 0; i < 3; i++)
	{
		ch = s[i];
		if (ch < '0' || ch > '9')
			break;
		num = num*10 + (ch - '0');
	}
	return num;
}
void guess()
{
	printf("              ***************************************************\n");
	printf("              *               Guess number                      *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *              Enter num to guess                 *\n");
	printf("              *              Enter e to quit                    *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
	int stop = 0;
	int a,b;
	char c;
	a = 543;
	printf("I have a number between 1 and 999.\nCan you guess my number?\nPlease type your first guess.\n");
	char bufr[128];
	read(0, bufr, 128);
	b=my_atoi(bufr);
	while (b!= -1)
	{
		if (b == a)
		{
			printf("Excellent! You guessed the number!\nWould you like to play again(y or n)?");
			char bur[128];
			read(0, bur, 128);
			switch (bur[0]) {
			case 'y':
				printf("I have a number between 1 and 1000.\nCan you guess my number?\nPlease type your first guess.\n");
				read(0, bufr, 128);
				break;
			case 'n':
				stop = 1;
				break;
			}
			if (stop == 1)
				break;
		}
		while (b<a)
		{
			printf("Too low.Try again.\n");
			read(0, bufr, 128);
			b=my_atoi(bufr);
		}
		while (b>a)
		{
			printf("Too high.Try again.\n");
			read(0, bufr, 128);
			b=my_atoi(bufr);
		}
		if(bufr[0]=='e')
			break;
	}
}

/*========================================================================*
							Gobang game
 *=========================================================================*/

#define N  9
int chessboard[N + 1][N + 1] = { 0 };

//用来记录轮到玩家1还是玩家2
int whoseTurn = 0;

void initGame(void);
void printChessboard(void);
void playChess(void);
int GobangJudge(int, int);
int stop = 0;

void Gobang()
{
	printf("              ***************************************************\n");
	printf("              *               Gobang Game                       *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *         Enter num+x+num+y to play               *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
	int i, j;
	for (i = 0; i <= N; i++)
		for (j = 0; j <= N; j++)
		{
			chessboard[i][j] = 0;
		}
	printChessboard();

	whoseTurn = 0;
	stop = 0;
	chessboard[N + 1][N + 1] = 0;
	printf("Player1:");
	while (1)
	{

		whoseTurn++;
		playChess();
		if (stop == 1)break;
	}
	stop = 0;
	return 0;
}

void printChessboard(void)
{
	int i, j;

	for (i = 0; i <= N; i++)
	{
		for (j = 0; j <= N; j++)
		{
			if (0 == i) printf("%3d", j);
			else if (j == 0) printf("%3d", i);
			else if (1 == chessboard[i][j]) printf("  F");
			else if (2 == chessboard[i][j]) printf("  T");
			else printf("  -");
		}
		printf("\n");
	}
}

void playChess(void)
{
	int i, j;
	char bufr[128];
	read(0, bufr, 128);

	if (1 == whoseTurn % 2)
	{

		i = bufr[0] - '0';
		j = bufr[2] - '0';
		chessboard[i][j] = 1;
	}
	if (0 == whoseTurn % 2)
	{

		i = bufr[0] - '0';
		j = bufr[2] - '0';
		chessboard[i][j] = 2;
	}

	if (bufr[0] == 'e') stop = 1;
	else
	{
		printChessboard();
		if (1 == whoseTurn % 2) printf("Player2:");
		else printf("Player1:");
	}
	if (GobangJudge(i, j))
	{
		if (1 == whoseTurn % 2) printf("Player1 win!\n");
		else printf("Player2 win!\n");
		stop = 1;
	}

}

int GobangJudge(int x, int y)
{
	int i, j;
	int t = 2 - whoseTurn % 2;

	for (i = x - 4, j = y; i <= x; i++)
	{
		if (i >= 1 && i <= N - 4 && t == chessboard[i][j] && t == chessboard[i + 1][j] && t == chessboard[i + 2][j] && t == chessboard[i + 3][j] && t == chessboard[i + 4][j])
			return 1;
	}
	for (i = x, j = y - 4; j <= y; j++)
	{
		if (j >= 1 && j <= N - 4 && t == chessboard[i][j] && t == chessboard[i][j + 1] && t == chessboard[i][j + 1] && t == chessboard[i][j + 3] && t == chessboard[i][j + 4])
			return 1;
	}
	for (i = x - 4, j = y - 4; i <= x, j <= y; i++, j++)
	{
		if (i >= 1 && i <= N - 4 && j >= 1 && j <= N - 4 && t == chessboard[i][j] && t == chessboard[i + 1][j + 1] && t == chessboard[i + 2][j + 2] && t == chessboard[i + 3][j + 3] && t == chessboard[i + 4][j + 4])
			return 1;
	}
	for (i = x + 4, j = y - 4; i >= 1, j <= y; i--, j++)
	{
		if (i >= 1 && i <= N - 4 && j >= 1 && j <= N - 4 && t == chessboard[i][j] && t == chessboard[i - 1][j + 1] && t == chessboard[i - 2][j + 2] && t == chessboard[i - 3][j + 3] && t == chessboard[i - 4][j + 4])
			return 1;
	}

	return 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						 push box
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int pi = 0;
int pj = 0;
void draw_map(int map[9][11])
{
	int i;
	int j;
	for (i = 0; i < 9; i++)
	{
		for (int j = 0; j < 11; j++)
		{
			switch (map[i][j])
			{
			case 0:
				printf(" "); //道路
				break;
			case 1:
				printf("#"); //墙壁
				break;
			case 2:
				printf(" "); //游戏边框的空白部分
				break;
			case 3:
				printf("D"); //目的地
				break;
			case 4:
				printf("b"); //箱子
				break;
			case 7:
				printf("!"); //箱子进入目的地
				break;
			case 6:
				printf("p"); //人
				break;
			case 9:
				printf("^"); //人进入目的地
				break;
			}
		}
		printf("\n");
	}
}
//void draw_map(int map[9][11]); 
//void boxMenu();
void boxMenu()
{
	printf("              ***************************************************\n");
	printf("              *               Gobang Game                       *\n");
	printf("              ***************************************************\n");
	printf("              *                                                 *\n");
	printf("              *               Enter sdwa to play                *\n");
	printf("              *                                                 *\n");
	printf("              ***************************************************\n\n");
}
void Runpushbox(fd_stdin, fd_stdout)
{
	char rdbuf[128];
	int r;
	char control;

	int count = 0;   //定义记分变量

	int map[9][11] = {
		{2,1,1,1,1,1,1,1,1,1,2},
		{2,1,0,0,0,1,0,0,0,1,2},
		{2,1,0,4,4,4,4,4,0,1,2},
		{2,1,0,4,0,4,0,4,0,0,1},
		{2,1,0,0,0,6,0,0,4,0,1},
		{1,1,0,1,1,1,1,0,4,0,1},
		{1,0,3,3,3,3,3,1,0,0,1},
		{1,0,3,3,3,3,3,0,0,0,1},
		{1,1,1,1,1,1,1,1,1,1,2},
	};
	while (1)
	{
		clear();
		printf("\n");
		boxMenu();
		draw_map(map);
		printf("Current Score:%d\n", count);
		//找初始位置
		for (pi = 0; pi < 9; pi++)
		{
			for (pj = 0; pj < 11; pj++)
			{
				if (map[pi][pj] == 6 || map[pi][pj] == 9)
					break;
			}
			if (map[pi][pj] == 6 || map[pi][pj] == 9)
				break;
		}
		printf("CURRENT LOCATION (%d,%d)", pi, pj);

		printf("\n");
		printf("Please input direction:");

		r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		control = rdbuf[0];


		if (control == 'Q' || control == 'q')
		{
			break;
		}
		switch (control)
		{
		case 'w':
			//如果人前面是空地。  
			if (map[pi - 1][pj] == 0)
			{
				map[pi - 1][pj] = 6 + 0;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			//如果人前面是目的地。
			else if ((map[pi - 1][pj] == 3) || (map[pi - 1][pj] == 9))
			{
				map[pi - 1][pj] = 6 + 3;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			//如果人前面是箱子。
			else if (map[pi - 1][pj] == 4)
			{
				if (map[pi - 2][pj] == 0)
				{
					map[pi - 2][pj] = 4;
					if (map[pi - 1][pj] == 7)
						map[pi - 1][pj] = 9;
					else
						map[pi - 1][pj] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				//如果人的前面是箱子，而箱子前面是目的地。
				else if (map[pi - 2][pj] == 3)
				{
					map[pi - 2][pj] = 7;
					count++;
					if (map[pi - 1][pj] == 7)
						map[pi - 1][pj] = 9;
					else
						map[pi - 1][pj] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			//如果人前面是已经进入某目的地的箱子
			else if (map[pi - 1][pj] == 7)
			{
				//如果人前面是已经进入某目的地的箱子,箱子前面是空地。
				if (map[pi - 2][pj] == 0)
				{
					count--;
					map[pi - 2][pj] = 4;
					map[pi - 1][pj] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				//如果人前面是已经进入某目的地的箱子，箱子前面是另一目的地。
				if (map[pi - 2][pj] == 3)
				{
					map[pi - 2][pj] = 7;
					map[pi - 1][pj] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			break;
		case 's':
			//如果人前面是空地。
			if (map[pi + 1][pj] == 0)
			{
				map[pi + 1][pj] = 6 + 0;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			//如果人前面是目的地。
			else if (map[pi + 1][pj] == 3)
			{
				map[pi + 1][pj] = 6 + 3;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			//如果人前面是箱子。
			else if (map[pi + 1][pj] == 4)
			{
				//如果人前面是箱子，而箱子前面是空地。
				if (map[pi + 2][pj] == 0)
				{
					map[pi + 2][pj] = 4;
					if (map[pi + 1][pj] == 7)
						map[pi + 1][pj] = 9;
					else
						map[pi + 1][pj] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				//如果人的前面是箱子，而箱子前面是目的地。
				else if (map[pi + 2][pj] == 3)
				{
					map[pi + 2][pj] = 7;
					count++;
					if (map[pi + 1][pj] == 7)
						map[pi + 1][pj] = 9;
					else
						map[pi + 1][pj] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			else if (map[pi + 1][pj] == 7)
			{
				if (map[pi + 2][pj] == 0)
				{
					count--;
					map[pi + 2][pj] = 4;
					map[pi + 1][pj] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				if (map[pi + 2][pj] == 3)
				{
					map[pi + 2][pj] = 7;
					map[pi + 1][pj] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			break;
		case 'a':
			if (map[pi][pj - 1] == 0)
			{
				map[pi][pj - 1] = 6 + 0;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			else if (map[pi][pj - 1] == 3)
			{
				map[pi][pj - 1] = 6 + 3;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			else if (map[pi][pj - 1] == 4)
			{
				if (map[pi][pj - 2] == 0)
				{
					map[pi][pj - 2] = 4;
					if (map[pi][pj - 1] == 7)
						map[pi][pj - 1] = 9;
					else
						map[pi][pj - 1] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				else if (map[pi][pj - 2] == 3)
				{
					count++;
					map[pi][pj - 2] = 7;
					if (map[pi][pj - 1] == 7)
						map[pi][pj - 1] = 9;
					else
						map[pi][pj - 1] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			else if (map[pi][pj - 1] == 7)
			{
				if (map[pi][pj - 2] == 0)
				{
					count--;
					map[pi][pj - 2] = 4;
					map[pi][pj - 1] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				if (map[pi][pj - 2] == 3)
				{
					map[pi][pj - 2] = 7;
					map[pi][pj - 1] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			break;
		case 'd':
			if (map[pi][pj + 1] == 0)
			{
				map[pi][pj + 1] = 6 + 0;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			else if (map[pi][pj + 1] == 3)
			{
				map[pi][pj + 1] = 6 + 3;
				if (map[pi][pj] == 9)
					map[pi][pj] = 3;
				else
					map[pi][pj] = 0;
			}
			else if (map[pi][pj + 1] == 4)
			{
				if (map[pi][pj + 2] == 0)
				{
					map[pi][pj + 2] = 4;
					if (map[pi][pj + 1] == 7)
						map[pi][pj + 1] = 9;
					else
						map[pi][pj + 1] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				else if (map[pi][pj + 2] == 3)
				{
					count++;
					map[pi][pj + 2] = 7;
					if (map[pi][pj + 1] == 7)
						map[pi][pj + 1] = 9;
					else
						map[pi][pj + 1] = 6;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			else if (map[pi][pj + 1] == 7)
			{
				if (map[pi][pj + 2] == 0)
				{
					count--;
					map[pi][pj + 2] = 4;
					map[pi][pj + 1] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
				if (map[pi][pj + 2] == 3)
				{
					map[pi][pj + 2] = 7;
					map[pi][pj + 1] = 9;
					if (map[pi][pj] == 9)
						map[pi][pj] = 3;
					else
						map[pi][pj] = 0;
				}
			}
			break;
		}
		if (count == 8)
		{
			draw_map(map);
			printf("\nCongratulations!!\n");
			break;    //退出死循环
		}
	}
}
 /*****************************************************************************
 *                                shell
 *****************************************************************************/
void shell(char *tty_name){
	 int fd;

    //int isLogin = 0;

    char rdbuf[512];
    char cmd[512];
    char arg1[512];
    char arg2[512];
    char buf[1024];
    char temp[512];
    char filename1[128];
    char filename2[128];
   
          
    

    int fd_stdin  = open(tty_name, O_RDWR);
    assert(fd_stdin  == 0);
    int fd_stdout = open(tty_name, O_RDWR);
    assert(fd_stdout == 1);

    //animation();
    clear(); 
    hello_cytuz();
    //animation_l();
    clear();
    displayWelcomeInfo();
   
    
   	

   char current_dirr[512] = "/";
   
    while (1) {  
        //清空数组中的数据用以存放新数据
        clearArr(rdbuf, 512);
        clearArr(cmd, 512);
        clearArr(arg1, 512);
        clearArr(arg2, 512);
        clearArr(buf, 1024);
        clearArr(temp, 512);
        
        

        printf("~%s$ ", current_dirr);

        int r = read(fd_stdin, rdbuf, 512);

        if (strcmp(rdbuf, "") == 0)
            continue;

        //解析命令
        int i = 0;
        int j = 0;
        while(rdbuf[i] != ' ' && rdbuf[i] != 0)
        {
            cmd[i] = rdbuf[i];
            i++;
        }
        i++;
        while(rdbuf[i] != ' ' && rdbuf[i] != 0)
        {
            arg1[j] = rdbuf[i];
            i++;
            j++;
        }
        i++;
        j = 0;
        while(rdbuf[i] != ' ' && rdbuf[i] != 0)
        {
            arg2[j] = rdbuf[i];
            i++;
            j++;
        }
        //清空缓冲区
        rdbuf[r] = 0;

        if (strcmp(cmd, "process") == 0)
        {
            ProcessManage();
        }
        else if (strcmp(cmd, "help") == 0)
        {
            help();
        }      
        else if (strcmp(cmd, "clear") == 0)
        {
            printTitle();
        }
        else if (strcmp(cmd, "ls") == 0)
        {
            ls(current_dirr);
        }
	else if (strcmp(cmd, "guess") == 0)
	{
		guess();
	}
	else if (strcmp(cmd, "gobang") == 0)
	{
		Gobang();
	}
	else if (strcmp(cmd, "calculator") == 0)
	{
		calculator();
	}
	else if (strcmp(cmd, "push") == 0)
		{
			Runpushbox(fd_stdin, fd_stdout);
		}
	else if (strcmp(cmd, "queen") == 0)
	{
		queen();
	}
	else if (strcmp(cmd, "toe") == 0)
	{
		toe();
	}
        else if(strcmp(rdbuf,"pause 4") == 0 )
        {
            proc_table[4].run_state = 0 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"pause 5") == 0 )
        {
            proc_table[5].run_state = 0 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"pause 6") == 0 )
        {
            proc_table[6].run_state = 0 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"kill 4") == 0 )
        {
//            proc_table[4].p_flags = 1;
//            ProcessManage();
            printf("cant kill this process!\n");
        }
        else if(strcmp(rdbuf,"kill 5") == 0 )
        {
            proc_table[5].p_flags = 1;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"kill 6") == 0 )
        {
            proc_table[6].p_flags = 1;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"resume 4") == 0 )
        {
            proc_table[4].run_state = 1 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"resume 5") == 0 )
        {
            proc_table[5].run_state = 1 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"resume 6") == 0 )
        {
            proc_table[6].run_state = 1 ;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"up 4") == 0 )
        {
            proc_table[4].priority = proc_table[4].priority*2;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"up 5") == 0 )
        {
            proc_table[5].priority = proc_table[5].priority*2;
            ProcessManage();
        }
        else if(strcmp(rdbuf,"up 6") == 0 )
        {
            proc_table[6].priority = proc_table[6].priority*2;
            ProcessManage();
        }
        else if (strcmp(cmd, "touch") == 0)
        {
            CreateFile(current_dirr, arg1);
        }
        else if (strcmp(cmd, "cat") == 0)
        {
            if(arg1[0]!='/'){
                addTwoString(temp,current_dirr,arg1);
                memcpy(arg1,temp,512);                
            }

            fd = open(arg1, O_RDWR);
            if (fd == -1)
            {
                printf("Failed to open file! Please check the filename!\n");
                continue ;
            }
            read(fd, buf, 1024);
            close(fd);
            printf("%s\n", buf);
        }
        else if (strcmp(cmd, "vi") == 0)
        {
            if(arg1[0]!='/'){
                addTwoString(temp,current_dirr,arg1);
                memcpy(arg1,temp,512);                
            }

            fd = open(arg1, O_RDWR);
            if (fd == -1)
            {
                printf("Failed to open file! Please check the filename!\n");
                continue ;
            }
            int tail = read(fd_stdin, rdbuf, 512);
            rdbuf[tail] = 0;

            write(fd, rdbuf, tail+1);
            close(fd);
        }
        else if (strcmp(cmd, "rm") == 0)
        {

            if(arg1[0]!='/'){
                addTwoString(temp,current_dirr,arg1);
                memcpy(arg1,temp,512);                
            }

            int result;
            result = unlink(arg1);
            if (result == 0)
            {
                printf("File deleted!\n");
                continue;
            }
            else
            {
                printf("Failed to delete file! Please check the filename!\n");
                continue;
            }
        } 
        else if (strcmp(cmd, "cp") == 0)
        {
            //首先获得文件内容
            if(arg1[0]!='/'){
                addTwoString(temp,current_dirr,arg1);
                memcpy(arg1,temp,512);                
            }
            fd = open(arg1, O_RDWR);
            if (fd == -1)
            {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
            
            int tail = read(fd, buf, 1024);
            close(fd);

            if(arg2[0]!='/'){
                addTwoString(temp,current_dirr,arg2);
                memcpy(arg2,temp,512);                
            }
            
            fd = open(arg2, O_CREAT | O_RDWR);
            if (fd == -1)
            {
                //文件已存在，什么都不要做
            }
            else
            {
                //文件不存在，写一个空的进去
                char temp2[1024];
                temp2[0] = 0;
                write(fd, temp2, 1);
                close(fd);
            }
             
            //给文件赋值
            fd = open(arg2, O_RDWR);
            write(fd, buf, tail+1);
            close(fd);
        } 
        else if (strcmp(cmd, "mv") == 0)
        {
             if(arg1[0]!='/'){
                addTwoString(temp,current_dirr,arg1);
                memcpy(arg1,temp,512);                
            }
            //首先获得文件内容
            fd = open(arg1, O_RDWR);
            if (fd == -1)
            {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
           
            int tail = read(fd, buf, 1024);
            close(fd);

            if(arg2[0]!='/'){
                addTwoString(temp,current_dirr,arg2);
                memcpy(arg2,temp,512);                
            }
            
            fd = open(arg2, O_CREAT | O_RDWR);
            if (fd == -1)
            {
                //文件已存在，什么都不要做
            }
            else
            {
                //文件不存在，写一个空的进去
                char temp2[1024];
                temp2[0] = 0;
                write(fd, temp2, 1);
                close(fd);
            }
             
            //给文件赋值
            fd = open(arg2, O_RDWR);
            write(fd, buf, tail+1);
            close(fd);
            //最后删除文件
            unlink(arg1);
        }   
        else if (strcmp(cmd, "dm") == 0)
        {
            fd = open(arg1, O_RDWR);
            if (fd == -1)
            {
                printf("File not exists! Please check the filename!\n");
                continue ;
            }
        } 
        else if (strcmp(cmd, "minesweeper") == 0){
        	runMine(fd_stdin, fd_stdout);
        }
        else if (strcmp(cmd, "game2048")==0){
        	Run2048(fd_stdin, fd_stdout);
        }
        else if (strcmp(cmd, "mkdir") == 0){
            CreateDir(current_dirr, arg1);          
        }
        else if (strcmp(cmd, "cd") == 0){
            if(arg1[0]!='/'){ // not absolute path from root
                i = j =0;
                while(current_dirr[i]!=0){
                    arg2[j++] = current_dirr[i++];
                }
                i = 0;
               
                while(arg1[i]!=0){
                    arg2[j++]=arg1[i++];
                }
                arg2[j++] = '/';
                arg2[j]=0;
                memcpy(arg1, arg2, 512);
		
            }
            else if(strcmp(arg1, "/")!=0){
                for(i=0;arg1[i]!=0;i++){}
                arg1[i++] = '/';
                arg1[i] = 0;
            }
            printf("%s\n", arg1);
	    int nump=0;
           char eachf[512];
		memset(eachf,0,sizeof(eachf)); 
		const char *s = arg1;
		char *t = eachf;
				if(arg1[0]=="/")
					*s++;
				while (*s) {	
					*t++ = *s++;
				}
		t = eachf;
		while (*t) {	
			char *ps = t+1;
			if(*ps==0){
				if(*t == '/')
				*t = 0;			
			}		
			t++;
		}	
            fd = open(eachf, O_RDWR);
            printf("%s\n", eachf);
            if(fd == -1){
                printf("The path not exists!Please check the pathname!\n");
            }
            else{
                memcpy(current_dirr, arg1, 512);
                printf("Change dir %s success!\n", current_dirr);
            }
        }
        else
            printf("Command not found, please check!\n");
            
    }
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{

	
	//while(1);
	char tty_name[] = "/dev_tty0";
	shell(tty_name);
	assert(0); /* never arrive here */
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	char tty_name[] = "/dev_tty1";
	shell(tty_name);
	
	assert(0); /* never arrive here */
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	//char tty_name[] = "/dev_tty2";
	//shell(tty_name);
	spin("TestC");
	/* assert(0); */
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

