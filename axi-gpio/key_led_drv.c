#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include <asm/arch/regs-gpio.h>
//#include <asm/hardware.h>
#include <linux/device.h>
//#include <mach/gpio.h>
#include <linux/interrupt.h>

#define		GPIO_CHAN1_TSR		0x04								// Channel 1 tri sate register offset
#define		GPIO_CHAN2_TSR		0x0C								// Channel 2 tri sate register offset
#define		GPIO_CHAN1_DR		0x00								// Channel 1 data register offset
#define		GPIO_CHAN2_DR		0x08								// Channel 2 data register offset
#define		GPIO_GIER			0x11C								// Global Interrupt Enable Register offset	
#define		GPIO_IER			0x128								// IP Interrupt Enable Register (IP IER) offset
#define		GPIO_ISR			0x120								// IP Interrupt Status Register offset

#define XGPIO_GIE_GINTR_ENABLE_MASK 0x80000000
#define IRQ_MASK 0x1

#define		IRQ_NUMBER			61								// IP Interrupt Status Register offset

static struct class *key_led_drv_class;

volatile unsigned long *gpled;

volatile unsigned long *gpkey;

static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

/* 中断事件标志, 中断服务程序将它置1，third_drv_read将它清0 */
static volatile int ev_press = 0;


/* 键值: 按下时, 0x01, 0x02, 0x03, 0x04 */
/* 键值: 松开时, 0x81, 0x82, 0x83, 0x84 */
static unsigned char key_val;

static void interrupt_clear(void)
{
        u32 val;
        val = *(gpkey + GPIO_ISR);
        printk("GPIO_ISR clear before %d\n",val);
	*(gpkey + GPIO_ISR) = val & IRQ_MASK;
        printk("after GPIO_ISR clear = %lu\n",*(gpkey + GPIO_ISR));
}
/*
  * 确定按键值
  */
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	key_val = 0xff;
	
	/*读取按键值*/	
	key_val = *(gpkey)|key_val ;
	
	printk("pin pressed! irq raised! key_val = %d\n",key_val);
	
	switch(key_val)
	{
		case 0x01:
			printk("pin pressed! irq raised! key_val = %d\n",key_val);
		case 0x10:
			printk("pin pressed! irq raised! key_val = %d\n",key_val);	
		case 0x11:
			printk("pin pressed! irq raised! key_val = %d\n",key_val);
		default:
			printk("irq raised? key_val = %d\n",key_val);
	}
	/*清中断*/
	interrupt_clear();

	ev_press = 1;                  /* 表示中断发生了 */
	wake_up_interruptible(&button_waitq);   /* 唤醒休眠的进程 */

	
	return IRQ_RETVAL(IRQ_HANDLED);
}

static void interrupt_enable(void)
{
	//from stand alone program
	//XGpio_InterruptEnable(&Gpio, BUTTON_INTERRUPT);
	//XGpio_InterruptGlobalEnable(&Gpio);
	u32 val;

	val = *(gpkey+GPIO_IER);
	printk("debug1 GPIO_IER val = %d\n",val);
	*(gpkey+GPIO_IER) = val | IRQ_MASK;

	*(gpkey + GPIO_GIER) = XGPIO_GIE_GINTR_ENABLE_MASK;	
	printk("debug2 read  GPIO_IER val = %lu\n",*(gpkey+GPIO_IER));
}

static int key_led_drv_open(struct inode *inode, struct file *file)
{
	/* 配置gpkey 0,1为输入引脚 */
	/* 配置gpled 0,1为输出引脚 */
	int result;

	/*AXI-GPIO初始化*/
	*(gpkey+GPIO_CHAN1_TSR) = (1<<0) | (1<<1);
	*(gpled+GPIO_CHAN1_TSR) = (0<<0) | (0<<1);
	printk("debug0 read  GPIO_CHAN1_TSR val = %lu\n",*(gpkey+GPIO_CHAN1_TSR));
    	printk("debug0 read  gpkey = %lu\n",*(gpkey));
	
	interrupt_enable();

	/*注册中断*/
	result = request_irq(IRQ_NUMBER,  buttons_irq, IRQ_TYPE_EDGE_RISING, "key_irq", NULL);
	if (result < 0) 
		printk("unable to request IRQ%d : %d\n", IRQ_NUMBER, result);
	else
		printk("request IRQ%d success : %d\n", IRQ_NUMBER, result);

	*gpled = 0x02;

	return 0;
}

static ssize_t key_led_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	/* 如果没有按键动作, 休眠 */
	wait_event_interruptible(button_waitq, ev_press);

	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, &key_val, 1);
	*gpled = key_val;

	ev_press = 0;
	
	return 1;
}


int key_led_drv_close(struct inode *inode, struct file *file)
{
	free_irq(IRQ_NUMBER, NULL);
	return 0;
}


static struct file_operations key_led_drv_fops = {
    	.owner   =  THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    	.open    =  key_led_drv_open,     
	.read	 =	key_led_drv_read,	   
	.release =  key_led_drv_close,	   
};


int major;
static int key_led_drv_init(void)
{
	major = register_chrdev(0, "key_led_drv", &key_led_drv_fops);

	key_led_drv_class = class_create(THIS_MODULE, "key_led_drv");

	device_create(key_led_drv_class, NULL, MKDEV(major, 0), NULL, "key_led"); /* /dev/buttons */

	gpled = (volatile unsigned long *)ioremap(0x41200000, sizeof(u32));

	gpkey = (volatile unsigned long *)ioremap(0x41210000, sizeof(u32));

	*gpled = 0x01;

	return 0;
}

static void key_led_drv_exit(void)
{
	unregister_chrdev(major, "key_led");
	device_destroy(key_led_drv_class, MKDEV(major, 0));
	class_destroy(key_led_drv_class);
	iounmap(gpled);
	iounmap(gpkey);
}


module_init(key_led_drv_init);

module_exit(key_led_drv_exit);

MODULE_LICENSE("GPL");



