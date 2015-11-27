KERN_DIR = /home/ctq/zc702_trd14.1_3.0/linux-xlnx3.0

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m   += pca954x_dev.o
obj-m   += pca954x_drv.o
