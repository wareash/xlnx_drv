#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/* i2c_test r addr
 * i2c_test w addr val
 */

void print_usage(char *file)
{
 	printf("**************************************************************************************************************************\n");
 	printf("**ID = 0x00:Si570 clock          ID = 0x01:ADV7511 HDMI      ID = 0x02:I2C EEPROM     ID = 0x03:I2C port expander       **\n");
	printf("**ID = 0x04:I2C real time clock  ID = 0x05:FMC LPC J3        ID = 0x06:FMC LPC J4     ID = 0x03:UCD9248 controller PMBUS**\n");	
 	printf("**************************************************************************************************************************\n");
	printf("%s r  \n", file);
	printf("%s w Val \n", file);
}

int main(int argc, char **argv)
{
	int fd;
	unsigned char buf[2];
	
	if ((argc != 2) && (argc != 3))
	{
		print_usage(argv[0]);
		return -1;
	}

	fd = open("/dev/pca954x", O_RDWR);
	if (fd < 0)
	{
		printf("can't open /dev/pca954x\n");
		return -1;
	}

	if (strcmp(argv[1], "r") == 0)
	{
		read(fd, buf, 1);
		printf("MUXID = :%d",buf[0]);

		switch (buf[0])
		{
			case 0: printf("  Si570 clock channel select !\n"); break;
			case 1: printf("  ADV7511 HDMI channel select\n"); break;
			case 2: printf("  I2C EEPROM channel select\n"); break;
			case 3: printf("  I2C port expander channel select\n"); break;
			case 4: printf("  I2C real time clock channel select\n"); break;
			case 5: printf("  FMC LPC J3 channel select\n"); break;
			case 6: printf("  FMC LPC J4 channel select\n"); break;
			case 7: printf("  UCD9248 controller PMBUS channel select\n"); break;
			default: print_usage(argv[0]); break;
		}
	}
	else if (strcmp(argv[1], "w") == 0)
	{
		buf[0] = strtoul(argv[2], NULL, 0);
		write(fd, buf, 1);
	}
	else
	{
		print_usage(argv[0]);
		return -1;
	}
	
	return 0;
}

