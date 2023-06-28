//加入代码实现需要的C标准库和文件
#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>
#include <string.h>
#include "linux/capability.h"

//KCS接口定义了一组I/O映射的通信寄存器
#define KCS_CMD_REG 	0xCA3   //cmd/status寄存器
#define KCS_DATA_REG	0xCA2	//data_in/data_out寄存器

//定义KCS控制码
#define GET_STATUS		0x60	//发送0x60到cmd/status寄存器获取KCS寄存器状态
#define WRITE_START		0x61	//发送0x61到cmd/status寄存器开始写入数据
#define WRITE_END		0x62	//发送0x62到cmd/status寄存器结束写入数据
#define READ			0x68	//发送0x68到data_in/data_out寄存器读取数据

//定义KCS状态码
#define IDLE_STATE		0		//KCS闲置状态寄存器读值
#define READ_STATE		0x40	//读取数据时状态寄存器的读值
#define WRITE_STATE		0x80	//写入数据时状态寄存器的读值
#define ERROR_STATE		0xC0	//出现错误是状态寄存器的读值

//延时函数
void timedelay(int ms)
{
	int x,y,k=0;
	for(x=100;x>0;x--)
	for(y=ms;y>0;y--)
	k++;
}

//等待IBF被清空
void WaitIBFClear()
{
	int IBFStatus;
	do{
		timedelay(100);
		IBFStatus = inb(KCS_CMD_REG);	//从命令寄存器读取数据直到IBF为1
	}while ((IBFStatus & 0x02) == 1);
}

//主动清空OBF
void ClearOBF()
{
	inb(KCS_DATA_REG); //BMC读取数据寄存器时，OBF就会被清空
}

//等待OBF被设置
void WaitOBFSet()
{
	int OBFStatus;
	do{
		timedelay(100);
		OBFStatus = inb(KCS_CMD_REG); //从命令寄存器读取数据直到OBF为0
	}while ((OBFStatus & 0x01) == 0);
}

//获取KCS接口状态
KCS_State()
{
	int KCSState;
	KCSState = inb(KCS_CMD_REG);
	//return KCSState;
	return (KCSState & 0xC0);
}

//读取数据
void Read()
{
	int i = 0;
	int state;
	int responseArray[100];	//设置足够长的接收数据的数组
	printf(" ");
	do
	{
		state = KCS_State();	//判断当前KCS接口状态，执行不同操作
		if(state == READ_STATE)
		{
			WaitOBFSet();	//如果是读状态，等待OBF被设置
			responseArray[i] = inb(KCS_DATA_REG); //获取数据寄存器的值
			state = KCS_State();
			if((i>1) && (state == READ_STATE)) //如果完成码不是0x00，表示输入的命令错误或者不支持该命令
			{
				if((responseArray[2]) & 0xff != 0)
				{
					printf("%02x\n ",responseArray[2]);
					printf("the command you input is not correct, please check it carefully!\n");
					exit(1);
				}
				printf("%02x ",responseArray[i]);
				printf("%02x\n ",responseArray[1]);
			}
			outb(READ,KCS_DATA_REG); //每次只能读取一个字节的数据，每次都需要发送READ控制码请求读取更多数据，直到数据接收完成
			i++;
		}
		else if((state | IDLE_STATE) == IDLE_STATE) //读取完所有数据后，KCS接口则为空闲状态
			{
				printf("\n");
				WaitOBFSet(); //等待OBF被设置
		     	responseArray[i] = inb(KCS_DATA_REG);	//Read dummy data byte from 数据寄存器
				exit(1);
			}
			else
       		{
	       		exit(0);
      		}
	} while (1);
}

//主函数，程序运行的入口
int main(int argc,char *argv[])	
{
	int i=0,res;
	unsigned char ReqCmdData = 0;
    int state;
	if (argc<3){	//KCS的命令最少需要3组
		return printf("Not KCS-to-BMC request message!\n");
	}
	res=iopl(3);	//设置当前进程的I/O级别为最大
	if (res < 0)
    {
      perror("iopl set error!");
        return -1;
    }

	WaitIBFClear();
	ClearOBF();
	outb(WRITE_START,KCS_CMD_REG);	//发送WRITE_START控制码到命令寄存器开始写入操作
	WaitIBFClear();
	state = KCS_State();
	if ((state == WRITE_STATE)|(state == ERROR_STATE))
	{
		ClearOBF();
		ReqCmdData = strtoul(argv[1],0,0);	//将输入的第一个数据转化为十六进制字符串
		ReqCmdData = ReqCmdData << 2;	//向左偏移两位
		outb(ReqCmdData,KCS_DATA_REG);	//将得到的数据写入数据寄存器
		WaitIBFClear();
		ClearOBF();

		if(argc-1 == 2) //如果输入的数据只有两个，接着就需要将WRITE_END控制码发送到命令寄存器准备结束写操作
		{
			outb(WRITE_END,KCS_CMD_REG);	//将WRITE_END控制码发送到命令寄存器
			WaitIBFClear();
			state = KCS_State();
			if(state = WRITE_STATE)
			{
				ClearOBF();
				ReqCmdData = strtoul(argv[2],0,0);
				outb(ReqCmdData,KCS_DATA_REG); //写入最后1byte数据
				WaitIBFClear();
				Read(); //开始读取BMC返回的数据
			}
			else
			{
				return 0;
			}
		}
		else
		{
			for (i=2; i<argc-1; i++)	//如果输入的数据大于2byte,可以循环接收数据直到最后1byte
			{
				ReqCmdData = strtoul(argv[i],0,0);
				outb(ReqCmdData,KCS_DATA_REG);
				WaitIBFClear();
				ClearOBF();
			}
			outb(WRITE_END,KCS_CMD_REG); //写完倒数第二byte是，将WRITE_END控制码发送到命令寄存器
			WaitIBFClear();
			state = KCS_State();
			if(state = WRITE_STATE)
			{
				ClearOBF();
				ReqCmdData = strtoul(argv[argc-1],0,0);		
				outb(ReqCmdData,KCS_DATA_REG);	//接收最后1byte数据
				WaitIBFClear();
				Read();	//开始写操作
			}
			else
			{
				return 0;
			}
		}
	}
	else
	{
		 printf("BMC is not prepared to receiving this message!\n");
		 exit(0);
	}
}


