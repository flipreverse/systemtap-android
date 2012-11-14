#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "bootimg.h"

#define WRITE_BUFF_SIZE 1024

unsigned char write_buff[WRITE_BUFF_SIZE];

static void extract_image(int pFD,const char *pName,int pStart, int pLength);

static void usage(const char *name)
{
	fprintf(stderr,"usage: %s [--dry-run] <bootimg>\n",name);
}

int main (int argc, const char *argv[])
{
	int fd = 0, sum = 0, pos = 0, length = 0, dry_run = 0;
	void *buff = NULL;
	const char *input_file = NULL;
	struct stat input;

	if (argc < 2 || argc > 3)
	{
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (argc == 3)
	{
		if (strcmp(argv[1],"--dry-run") != 0)
		{
			usage(argv[0]);
			return EXIT_FAILURE;
		}
		input_file = argv[2];
		dry_run = 1;
	}
	else
	{
		input_file = argv[1];
	}

	buff = malloc(sizeof(boot_img_hdr));
	if (buff == NULL)
	{
		perror("malloc");
		return EXIT_FAILURE;
	}

	if (stat(input_file,&input) < 0)
	{
		perror("stat");
		return EXIT_FAILURE;
	}

	fd = open(input_file,O_RDONLY);
	if (fd < 0)
	{
		perror("open");
		return EXIT_FAILURE;
	}

	if (read(fd,buff,sizeof(boot_img_hdr)) < 0)
	{
		perror("read");
		return EXIT_FAILURE;
	}

	boot_img_hdr *foo = (boot_img_hdr*)buff;

	sum = 1 + (foo->kernel_size + foo->page_size - 1) / foo->page_size + (foo->ramdisk_size + foo->page_size - 1) / foo->page_size + (foo->second_size + foo->page_size - 1) / foo->page_size;
	sum *= foo->page_size;

	printf("Magic value:%.*s\n",BOOT_MAGIC_SIZE,foo->magic);
	printf("Kernel:\t\tsize:%u\t\taddr:0x%x\n",foo->kernel_size,foo->kernel_addr);
	printf("Ramdisk:\tsize:%u\t\taddr:0x%x\n",foo->ramdisk_size,foo->ramdisk_addr);
	printf("Second:\t\tsize:%u\t\t\taddr:0x%x\n",foo->second_size,foo->second_addr);
	printf("Page size:\t\t%u\n",foo->page_size);
	printf("Name:\t\t%.*s\n",BOOT_NAME_SIZE,foo->name);
	printf("Cmd:\t\t%.*s\n",BOOT_ARGS_SIZE,foo->cmdline);
	printf("Calc file size:%u\t\tstat size:%lu\n",sum,input.st_size);

	if (dry_run)
	{
		return EXIT_SUCCESS;
	}

	if (foo->kernel_size > 0)
	{
		pos = 1;
		pos *= foo->page_size;
		length = foo->kernel_size;

		printf("Extracting kernel (%u bytes)...\n",foo->kernel_size);
		extract_image(fd,"kernel.img",pos,length);
	}
	else
	{
		printf("No kernel image found.\n");
	}

	if (foo->ramdisk_size > 0)
	{
		pos = 1 + (foo->kernel_size + foo->page_size - 1) / foo->page_size;
		pos *= foo->page_size;
		length = foo->ramdisk_size;

		printf("Extracting ramdisk (%u bytes)...\n",foo->ramdisk_size);
		extract_image(fd,"ramdisk.img",pos, length);
	}
	else
	{
		printf("No ramdisk image found.\n");
	}

	if (foo->second_size > 0)
	{
		pos = 1 + (foo->kernel_size + foo->page_size - 1) / foo->page_size + (foo->ramdisk_size + foo->page_size - 1) / foo->page_size;
		pos *= foo->page_size;
		length = foo->second_size;

		printf("Extracting second (%u bytes)...\n",foo->second_size);
		extract_image(fd,"second.img",pos, length);
	}
	else
	{
		printf("No secondary image found.\n");
	}

	if (close(fd) < 0)
	{
		perror("close");
		return EXIT_FAILURE;
	}
	

	return EXIT_SUCCESS;
}

static void extract_image(int pFD,const char *pName,int pStart, int pLength)
{
	int out_fd = 0, pos = 0, bytes = 0, length = 0, to_read = 0;

	out_fd = open(pName,O_WRONLY|O_CREAT|O_TRUNC,0644);
	if (out_fd < 0)
	{
		perror("open output");
		exit(EXIT_FAILURE);
	}

	pos = pStart;
	length = pLength;

	if (lseek(pFD,pos,SEEK_SET) == (off_t)-1)
	{
		perror("lseek output");
		exit(EXIT_FAILURE);
	}

	while (length > 0)
	{
		if (length > WRITE_BUFF_SIZE)
		{
			to_read = WRITE_BUFF_SIZE;
		}
		else
		{
			to_read = length;
		}

		if ((bytes = read(pFD,write_buff,to_read)) < 0)
		{
			perror("read boot img");
			exit(EXIT_FAILURE);
		}
		length -= bytes;
		if (write(out_fd,write_buff,bytes) < 0)
		{
			perror("write output");
			exit(EXIT_FAILURE);
		}
	}	

	if (close(out_fd) < 0)
	{
		perror("close output");
		exit(EXIT_FAILURE);
	}
}
