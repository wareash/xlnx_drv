KERN_DIR = /home/ctq/trd2014_4/software/petalinux/build/linux/kernel/download/linux-xlnx

all:
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order

obj-m	+= ker_rw.o
