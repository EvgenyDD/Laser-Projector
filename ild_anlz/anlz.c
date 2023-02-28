#include "ilda.h"
#include <dirent.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FREE_PTR(x) \
	if(x)           \
	free(x)

#define S(x)                             \
	if((x))                              \
	{                                    \
		printf("[E]\t%s failed!\n", #x); \
		break;                           \
	}

static int get_buf_from_file(const char *f_name, char **p_buf, size_t *buf_size)
{
	FILE *fp = fopen(f_name, "r");
	// printf("[I]\tReading file '%s'...\n", f_name);
	if(!fp)
	{
		printf("[E]\tfopen (%s) failed!\n", f_name);
		return 1;
	}
	if(fseek(fp, 0L, SEEK_END) != 0)
	{
		printf("[E]\tfseek (%s) failed!\n", f_name);
		return 2;
	}
	long file_size = ftell(fp);
	if(file_size <= 0)
	{
		printf("[W]\tfile (%s) size is %ld!\n", f_name, file_size);
		return 3;
	}

	*p_buf = !*p_buf
				 ? (char *)malloc(file_size * sizeof(char))
				 : (char *)realloc(*p_buf, (*buf_size + file_size) * sizeof(char));
	if(!*p_buf)
	{
		printf("[E]\talloc failed!\n");
		return 4;
	}
	if(fseek(fp, 0L, SEEK_SET) != 0)
	{
		printf("[E]\tfseek failed!\n");
		return 5;
	}
	long new_len = fread(*p_buf + *buf_size, sizeof(char), file_size, fp);
	if(ferror(fp) != 0)
	{
		fputs("Error reading file", stderr);
		return 6;
	}
	if(new_len != file_size)
	{
		printf("[E]\tfile len read mismatch!\n");
		return 4;
	}
	*buf_size += file_size;

	return 0;
}

int main(int argc, char **argv)
{
	struct dirent *dir;
	DIR *d = opendir(argv[1]);
	if(d)
	{
		while((dir = readdir(d)) != NULL)
		{
			if(strstr(dir->d_name, ".ild"))
			{
				do
				{
					char *buf = NULL;
					size_t buf_size = 0;
					S(argc == 1);
					char fname[1024];
					strcpy(fname, argv[1]);
					strcat(fname, dir->d_name);
					S(get_buf_from_file(fname, &buf, &buf_size));
					printf("File %s: %d bytes ", fname, buf_size);
					ilda_t i;
					int sts = ilda_file_read(buf, buf_size, &i, true * 0);
					// if(sts == 0) printf("64-color fmt\n");
					if(sts == -4) sts = ilda_file_read(buf, buf_size, &i, false);
					if(sts != 0) printf("Failed: %d\n", sts);
					FREE_PTR(buf);
					ilda_file_free(&i);
				} while(0);
			}
		}
		closedir(d);
	}
	return 0;
}