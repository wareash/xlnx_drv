#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


static int major;
static struct class *pca954x_class;

static const struct i2c_client * pca954x_client; 


/*
 *传入buf[0] :addr
 *输出的数据buf :data
 */

static ssize_t pca954x_read (struct file *file, char __user *buf, size_t count, loff_t *off)
{
	unsigned char  data;
	 
//	copy_from_user(&addr,buf,2);

	data = i2c_smbus_read_byte(pca954x_client);
	printk("printk  read data = 0x%02x\n",data);
	copy_to_user(buf, &data, 1);
	return 1;
	
}


/*
 *buf[0] :addr
 *buf[1] :data
 */
static ssize_t pca954x_write (struct file *file, const char __user *buf, size_t count, loff_t *off)
{
	unsigned char ker_buf[1];
	unsigned char data;

	copy_from_user(ker_buf,buf,1);
	data = ker_buf[0];
	
	printk("printk  write data = 0x%02x\n",data);

	if(!i2c_smbus_write_byte(pca954x_client, data))
		return 1;
	else
		return -EIO;

	
}

static struct file_operations pca954x_fops={
		.owner 	= THIS_MODULE,
		.write 	= pca954x_write,
		.read 	= pca954x_read,
};

static const struct i2c_device_id pca954x_id_table[] = {
	{ "pca9548", 0 },
	{}
};


static int  pca954x_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	//printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__ );

	pca954x_client = client;

	major = register_chrdev( 0, "pca954x", &pca954x_fops);
	pca954x_class = class_create(THIS_MODULE, "pca954x");
	device_create(pca954x_class,NULL,MKDEV(major, 0), NULL,"pca954x");
	
	return 0;
}
static int  pca954x_remove(struct i2c_client *client)
{
	//printk("%s %s %d\n",__FILE__, __FUNCTION__, __LINE__ );
	device_destroy(pca954x_class,MKDEV(major, 0));
	class_destroy(pca954x_class);
	unregister_chrdev(major,"pca954x");
	return 0;
}


static struct i2c_driver pca954x_driver = {
	.class = I2C_CLASS_HWMON,  /*表示去哪些适配器上面去寻找设备*/
	.driver	= {
		.name	= "ctq_designed",
		.owner	= THIS_MODULE,
	},
	.probe		= pca954x_probe,
	.remove		= pca954x_remove,
	.id_table	= pca954x_id_table,
};


/*1. 分配/设置i2c_driver*/
static int pca954x_drv_init(void)
{
	/*2. 注册i2c_driver */
	
	printk("pca954x init  \n");
	i2c_add_driver(&pca954x_driver);
	
	return 0;
}
static void pca954x_drv_exit(void)
{
	 i2c_del_driver(&pca954x_driver);
}
module_init(pca954x_drv_init);
module_exit(pca954x_drv_exit);
MODULE_LICENSE("GPL");
	
