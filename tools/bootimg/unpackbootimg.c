/************************************************************************************************************/
/*	Copied from https://github.com/CyanogenMod/android_system_core/tree/cm-10.2/mkbootimg/unpackbootimg.c	*/
/*	Used the source code as it is except the parameter "--dry-run". This was added by the author of this	*/
/* 	project.																								*/
/************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <libgen.h>

#include "sha.h"
#include "bootimg.h"

typedef unsigned char byte;

int read_padding(FILE* f, unsigned itemsize, int pagesize)
{
    byte* buf = (byte*)malloc(sizeof(byte) * pagesize);
    unsigned pagemask = pagesize - 1;
    unsigned count;

    if((itemsize & pagemask) == 0) {
        free(buf);
        return 0;
    }

    count = pagesize - (itemsize & pagemask);

    fread(buf, count, 1, f);
    free(buf);
    return count;
}

void write_string_to_file(char* file, char* string)
{
    FILE* f = fopen(file, "w");
    fwrite(string, strlen(string), 1, f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

int usage() {
    printf("usage: unpackbootimg\n");
    printf("\t-i|--input boot.img\n");
    printf("\t[ -o|--output output_directory]\n");
    printf("\t[ -p|--pagesize <size-in-hexadecimal> ]\n");
    printf("\t[ --dry-run ]\n");
    return 0;
}

int main(int argc, char** argv)
{
    char tmp[PATH_MAX];
    char* directory = "./";
    char* filename = NULL;
    int pagesize = 0;
    byte dry_run = 0;
    byte param_found = 0;

    argc--;
    argv++;
    while(argc > 0){
        char *arg = argv[0];
        
        // First check for parameters without expecting values
        if(!strcmp(arg, "--dry-run")) {
            dry_run = 1;
            param_found = 1;
        }
        if (param_found == 1) {
            argc --;
            argv ++;
            param_found = 0;
            continue;
		}
        
        // Second check for parameters expecting values
        char *val = argv[1];
        argc -= 2;
        argv += 2;
        if(!strcmp(arg, "--input") || !strcmp(arg, "-i")) {
            filename = val;
        } else if(!strcmp(arg, "--output") || !strcmp(arg, "-o")) {
            directory = val;
        } else if(!strcmp(arg, "--pagesize") || !strcmp(arg, "-p")) {
            pagesize = strtoul(val, 0, 16);
        } else {
            return usage();
        }
    }
    
    if (filename == NULL) {
        return usage();
    }
    
    int total_read = 0;
    FILE* f = fopen(filename, "rb");
    boot_img_hdr header;

    printf("Reading header... ");
    int i;
    for (i = 0; i <= 512; i++) {
        fseek(f, i, SEEK_SET);
        fread(tmp, BOOT_MAGIC_SIZE, 1, f);
        if (memcmp(tmp, BOOT_MAGIC, BOOT_MAGIC_SIZE) == 0)
            break;
    }
    total_read = i;
    if (i > 512) {
        printf("Android boot magic not found.\n");
        return 1;
    }
    fseek(f, i, SEEK_SET);
    printf("Android magic found at offset: %d\n", i);

    fread(&header, sizeof(header), 1, f);
    
    if (pagesize == 0) {
        pagesize = header.page_size;
    }

	printf("Kernel:\t\t\tsize:%u\t\taddr:0x%x\n",header.kernel_size,header.kernel_addr);
	printf("Kernelbaseaddress:\t\t\t\taddr:0x%x\n",header.kernel_addr - 0x00008000);
	printf("Ramdisk:\t\tsize:%u\t\taddr:0x%x\n",header.ramdisk_size,header.ramdisk_addr);
	printf("Second:\t\t\tsize:%u\t\t\taddr:0x%x\n",header.second_size,header.second_addr);
	printf("Page:\t\t\tsize:%u\n",header.page_size);
	printf("Name:\t\t%.*s\n",BOOT_NAME_SIZE,header.name);
	printf("Cmd:\t\t%.*s\n",BOOT_ARGS_SIZE,header.cmdline);
	
    if (dry_run) {
		return 0;
	}
    
    //printf("cmdline...\n");
    sprintf(tmp, "%s/%s", directory, basename(filename));
    strcat(tmp, "-cmdline");
    write_string_to_file(tmp, header.cmdline);
    
    //printf("base...\n");
    sprintf(tmp, "%s/%s", directory, basename(filename));
    strcat(tmp, "-base");
    char basetmp[200];
    sprintf(basetmp, "%08x", header.kernel_addr - 0x00008000);
    write_string_to_file(tmp, basetmp);

    //printf("pagesize...\n");
    sprintf(tmp, "%s/%s", directory, basename(filename));
    strcat(tmp, "-pagesize");
    char pagesizetmp[200];
    sprintf(pagesizetmp, "%d", header.page_size);
    write_string_to_file(tmp, pagesizetmp);
    
    total_read += sizeof(header);
    total_read += read_padding(f, sizeof(header), pagesize);

	if (header.kernel_size > 0 ) {
		printf("Extracting kernel (%u bytes)...\n",header.kernel_size);
		sprintf(tmp, "%s/%s", directory, basename(filename));
		strcat(tmp, "-zImage");
		
		FILE *k = fopen(tmp, "wb");
		byte* kernel = (byte*)malloc(header.kernel_size);
		fread(kernel, header.kernel_size, 1, f);
		total_read += header.kernel_size;
		fwrite(kernel, header.kernel_size, 1, k);
		fclose(k);
	} else {
		printf("No kernel image found.\n");
	}
    total_read += read_padding(f, header.kernel_size, pagesize);

	if (header.ramdisk_size > 0) {
		printf("Extracting ramdisk (%u bytes)...\n",header.ramdisk_size);
		sprintf(tmp, "%s/%s", directory, basename(filename));
		strcat(tmp, "-ramdisk.gz");
		
		FILE *r = fopen(tmp, "wb");
		byte* ramdisk = (byte*)malloc(header.ramdisk_size);
		fread(ramdisk, header.ramdisk_size, 1, f);
		total_read += header.ramdisk_size;
		fwrite(ramdisk, header.ramdisk_size, 1, r);
		fclose(r);
	} else {
		printf("No ramdisk image found.\n");
	}
	total_read += read_padding(f, header.ramdisk_size, pagesize);
    
    if (header.second_size > 0){
		printf("Extracting second (%u bytes)...\n",header.second_size);
		sprintf(tmp, "%s/%s", directory, basename(filename));
		strcat(tmp, "-secimg");
		
		FILE *r = fopen(tmp, "wb");
		byte* ramdisk = (byte*)malloc(header.second_size);
		fread(ramdisk, header.second_size, 1, f);
		total_read += header.second_size;
		fwrite(ramdisk, header.second_size, 1, r);
		fclose(r);
	} else {
		printf("No secondary image found.\n");
	}
	total_read += read_padding(f, header.second_size, pagesize);
    
    fclose(f);
    
    printf("Total Read: %d\n", total_read);
    return 0;
}
