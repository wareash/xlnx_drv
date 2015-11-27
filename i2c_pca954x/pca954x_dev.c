#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/slab.h>



static struct i2c_client * pca954x_client;

static const unsigned short addr_list[] = {0x60 , 0x74 , I2C_CLIENT_END};

static int pca954x_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info pca954x_info;

	memset(&pca954x_info, 0, sizeof(struct i2c_board_info));
	strlcpy(pca954x_info.type,"pca9548",I2C_NAME_SIZE);
	
	i2c_adap = i2c_get_adapter(0);
	pca954x_client = i2c_new_probed_device(i2c_adap, &pca954x_info, addr_list, NULL);
	i2c_put_adapter(i2c_adap);

	if (pca954x_client)
		{
			printk("pca954x_client init success !\n");
			return 0;
		}
	else
		{
			printk("pca954x_client init fail !\n");
			return -ENODEV;
		}
}
static void pca954x_dev_exit(void)
{
	i2c_unregister_device(pca954x_client);
}

module_init(pca954x_dev_init);
module_exit(pca954x_dev_exit);
MODULE_LICENSE("GPL");


