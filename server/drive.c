#include "drive.h"

static struct class *driver_class;		//

static int major = 0;					//

volatile unsigned long *gppcon = NULL;				//8, 13
volatile unsigned long *gppdat = NULL;

volatile unsigned long *gpqcon = NULL;				//1, 3
volatile unsigned long *gpqdat = NULL;

void wait(void)
{
	volatile long dly;
	for (dly = 2000000; dly > 0; dly--);
}

static int car_driver_open(struct inode *inode, struct file *file)
{
	printk("open car!\n");
	//gppcon
	*gppcon &= ~((0x3 << (8 * 2)) | (0x3 << (13 * 2)));
	*gppcon |= ((0x1 << (8 * 2)) | (0x1 << (13 *2)));

	*gpqcon &= ~((0x3 << (2 * 2)) | (0x3 << (3 * 2)));
	*gpqcon |= ((0x1 << (2 * 2)) | (0x1 << (3 * 2)));

	printk("GPPCON=%X\n",ioread32(gppcon));
	printk("GPPDAT=%X\n",ioread32(gppdat));
	printk("gpqcon=%X\n",ioread32(gpqcon));
	printk("gpqdat=%X\n",ioread32(gpqdat));
	return 0;
}

static ssize_t car_driver_write(struct file *file, const char __user *buf,
							size_t count, loff_t *ppos)
{
	//

	int val = 0;
	
	copy_from_user(&val, buf, count);				//不够明白

	printk("驱动里面:val= %c\n", val);
	switch(val)
	{
		//前
		case 'w':
			*gppdat &= ~((0x1 << 8) | (0x1 << 13));		//1:0有电压差才转动
			*gppdat |= ((0x0 << 8) | (0x1 << 13));
			
			*gpqdat &= ~((0x1 << 2) | (0x1 << 3));		//1:0
			*gpqdat |= ((0x0 << 2) | (0x1 << 3));
			
			printk("qudong w\n");
			printk("GPPCON=%X\n",ioread32(gppcon));
			printk("GPPDAT=%X\n",ioread32(gppdat));
			printk("gpqcon=%X\n",ioread32(gpqcon));
			printk("gpqdat=%X\n",ioread32(gpqdat));
//			while(1);			////
			wait();										//
			break;
		
		//后
		case 's':
			*gppdat &= ~((0x1 << 8) | (0x1 << 13));		//1:0有电压差才转动
			*gppdat |= ((0x1 << 8) | (0x0 << 13));
			
			*gpqdat &= ~((0x1 << 2) | (0x1 << 3));		//1:0
			*gpqdat |= ((0x1 << 2) | (0x0 << 3));
			printk("qudong s\n");
			wait();										//
			break;
		
		//右
		case 'd':
			*gppdat &= ~((0x1 << 8) | (0x1 << 13));		//1:0有电压差才转动
			*gppdat |= ((0x1 << 8) | (0x0 << 13));
			
			*gpqdat &= ~((0x1 << 2) | (0x1 << 3));		//1:0
			*gpqdat |= ((0x0 << 2) | (0x1 << 3));
			printk("qudong a\n");
			wait();
			break;
		
		//左
		case 'a':
			*gppdat &= ~((0x1 << 8) | (0x1 << 13));		//1:0有电压差才转动
			*gppdat |= ((0x0 << 8) | (0x1 << 13));
			
			*gpqdat &= ~((0x1 << 2) | (0x1 << 3));		//1:0
			*gpqdat |= ((0x1 << 2) | (0x0 << 3));
			printk("qudong d\n");
			wait();
			break;
		
		//退出
		case 'q':
			*gppdat |= ((0x1 << 8) | (0x1 << 13));
			*gpqdat |= ((0x1 << 2) | (0x1 << 3));
			break;
		
		//退出程序
		case 'l':
			*gppdat |= ((0x1 << 8) | (0x1 << 13));
			*gpqdat |= ((0x1 << 2) | (0x1 << 3));
			break;
		
		default:
			break;

	}
	
	
	*gppdat |= ((0x1 << 8) | (0x1 << 13));
	*gpqdat |= ((0x1 << 2) | (0x1 << 3));
	
	return 0;
}


static int __init car_driver_init(void)					//落了__init
{
	printk("car_driver_init!\n");
	major = register_chrdev(0, "car", &driver_fops);			//注册

	driver_class = class_create(THIS_MODULE, "driver_class");
	device_create(driver_class, NULL, MKDEV(major, 0), NULL, "car");

//	request_mem_region(0x7F008180,8,"car");//申请内存	///////////
	
	//gppcon, gppdat
	gppcon = (volatile unsigned long *)ioremap(0x7F008160, 16);
	gppdat = gppcon + 1;

	//gpqcon, gppdat
	gpqcon = (volatile unsigned long *)ioremap(0x7F008180, 16);
	gpqdat = gpqcon + 1;											

	return 0;

}

static void __exit car_driver_exit(void)				//落了__exit
{
	printk("car_driver_exit!\n");
	unregister_chrdev(major, "car");
	device_destroy(driver_class, MKDEV(major, 0));
	class_destroy(driver_class);

	iounmap(gppcon);
	iounmap(gpqcon);											//解除映射

}

module_init(car_driver_init);
module_exit(car_driver_exit);

MODULE_LICENSE("GPL");
