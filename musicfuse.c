/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/
#define FUSE_USE_VERSION 31
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#define _GNU_SOURCE
#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

static const char *dirpath = "/home/bonnis/testing";
static const char *setpath = "/home/bonnis/tmusictest";

// int testdoang(const char * path,void* buf,fuse_fill_dir_t filler){
//     DIR *dp;
// 	struct dirent *de;
//     int res=0;

// 	dp = opendir(path);
// 	if (dp == NULL)
// 		return -errno;

//      while ((de = readdir(dp)) != NULL) {
// 		struct stat st;
// 		memset(&st, 0, sizeof(st));
// 		st.st_ino = de->d_ino;
// 		st.st_mode = de->d_type;
// 		res = (filler(buf, de->d_name, &st, 0));
// 			if(res!=0) break;
// 	}
// }

int copy(const char *file1, const char *file2){
    FILE *fptr1, *fptr2; 
    char c; 
  
    // Open one file for reading 
    fptr1 = fopen(file1, "r"); 
    if (fptr1 == NULL) 
    { 
        printf("Cannot open file %s \n", file1); 
        exit(0); 
    } 
  
    // Open another file for writing 
    fptr2 = fopen(file2, "w"); 
    if (fptr2 == NULL) 
    { 
        printf("Cannot open file %s \n", file2); 
        exit(0); 
    } 
  
    // Read contents from file 
    c = fgetc(fptr1); 
    while (c != EOF) 
    { 
        fputc(c, fptr2); 
        c = fgetc(fptr1); 
    } 
  
    printf("\nContents copied to %s", file2); 
  
    fclose(fptr1); 
    fclose(fptr2); 
    return 0; 
}

int recursive(const char *path)
{
    DIR *d = opendir(path);
   size_t path_len = strlen(path);
   int r = -1;

   if (d)
   {
      struct dirent *p;

      r = 0;

      while (p=readdir(d))
      {
          int r2 = -1, res;
          char *buf;
          size_t len;

          /* Skip the names "." and ".." as we don't want to recurse on them. */
          if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
          {
             continue;
          }

          len = path_len + strlen(p->d_name) + 2; 
          buf = malloc(len);
            printf("masuk\n");
          if (buf)
          {
             struct stat statbuf;

             snprintf(buf, len, "%s/%s", path, p->d_name);

             if (!stat(buf, &statbuf))
             {
                if (S_ISDIR(statbuf.st_mode))
                {
                    recursive(buf);
                }
                else
                {
                    char temporarystring[300];
                    char finalstring[1000];
                    char final2string[1000];
                    strcpy(temporarystring, p->d_name);
                    sprintf(finalstring,"%s/%s",path,temporarystring);
                    sprintf(final2string, "%s/%s",setpath,temporarystring);
                    // if(!strcmp((&temporarystring[strlen(temporarystring)-4]),".mp3"))
                    // {
                        printf("%s <<<<DEBUG\n",finalstring);
                        printf("%s <<<<DEBUG2\n",final2string);
                        copy(finalstring, final2string);
                    // }
                }
             }

             free(buf);
          }

        //   r = r2;
      }
      closedir(d);
    }   
}

static void *xmp_init(struct fuse_conn_info *conn,
                      struct fuse_config *cfg)
{
    (void) conn;
    recursive(dirpath);
    return NULL;
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
  int res;
	char fpath[1000];
	sprintf(fpath,"%s%s",dirpath,path);
	res = lstat(fpath, stbuf);

	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;

	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type;
		res = (filler(buf, de->d_name, &st, 0));
			if(res!=0) break;
	}

	closedir(dp);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
  char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else sprintf(fpath, "%s%s",dirpath,path);
	int res = 0;
  int fd = 0 ;

	(void) fi;
	fd = open(fpath, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.read		= xmp_read,
    .init       = xmp_init
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}