/*
*文件名：led.s
*
*
*
*/
#define GPJ0CON  0xE0200240
#define GPJ0DAT  0xE0200244
#define LED3456_ON 0x0
#define LED3456_OFF 0xFF
#define LED3_OFF  (1<<3)
#define LED4_OFF  (1<<4)
#define LED5_OFF  (1<<5)

#define GPD0CON  0xE02000A0
#define GPD0DAT  0xE02000A4
#define LED6_OFF (1<<1)

#define PWM (1<<2)

.global  _start
_start:
//把所有引脚都设置为输出模式，代码不变
	ldr r0, =0x11111111
    ldr r1, =GPJ0CON
	
	str r0,[r1]

//点亮GPD0_1这颗LED6
    ldr r4, = 0x1111
	ldr r5, = GPD0CON
	str r4,[r5]

	ldr r4, = LED3456_OFF
	ldr r5, = GPD0DAT
	str r4,[r5]

flash:

    //蜂鸣器
    ldr r4, = 6
    ldr r5, = GPD0DAT
    str r4,[r5]
	
    //第一步  LED3亮
	ldr r0, = LED3456_ON | LED4_OFF | LED5_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
    //第二步  延时
    bl dely
    //第三步  灭
    ldr r0, = LED3456_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
	//第四步  延时
    bl dely
    
    //第一步  LED4亮
    ldr r0, = LED3456_ON | LED3_OFF | LED5_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
    //第二步  延时
    bl dely
    //第三步  灭
    ldr r0, = LED3456_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
	//第四步  延时
	bl dely

	//第一步 LED5亮
    ldr r0, = LED3456_ON | LED3_OFF | LED4_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
    //第二步  延时
    bl dely
    //第三步  灭
    ldr r0, = LED3456_OFF
	ldr r1, =GPJ0DAT
	str r0,[r1]
	//第四步  延时
	bl dely

    //LED6 亮
    ldr r4, = LED3456_ON
	ldr r5, = GPD0DAT
	str r4,[r5]
    //第二步  延时
    bl dely
    //第三步  灭
    ldr r4, = LED6_OFF
	ldr r5, = GPD0DAT
	str r4,[r5]
	//第四步  延时
	bl dely
		
	b flash

//延时函数

dely:
     ldr r3, = 7777777
	 ldr r4, = 0x0
dely_loop:
     sub r3, r3, #1
	 cmp r3,r4
	 bne dely_loop
	 mov pc,lr
	 