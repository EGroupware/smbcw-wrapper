//Compile with gcc -o smbcw_test smbcw_test.c -lsmbcw -g
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smbcw.h>
#include <sys/stat.h>

#define TEST_URL "smb://smb_user:smb_user@localhost/smb_test/foo.test"
#define DIR_URL "smb://smb_user:smb_user@localhost/smb_test/"

char content[] = "Dies ist nur ein kleiner Test2\n";

void format_mode(int mode)
{
  if (mode & 0040000) printf("d"); else printf("-");
  if (mode & 00400) printf("r"); else printf("-");
  if (mode & 00200) printf("w"); else printf("-");
  if (mode & 00100) printf("x"); else printf("-");
  if (mode & 0040) printf("r"); else printf("-");
  if (mode & 0020) printf("w"); else printf("-");
  if (mode & 0010) printf("x"); else printf("-");
  if (mode & 004) printf("r"); else printf("-");
  if (mode & 002) printf("w"); else printf("-");
  if (mode & 001) printf("x"); else printf("-");
}

int main ()
{
  int fd = 0;

  //Open a file using smbcw
  fd = smbcw_fopen(TEST_URL, "w");
  if (fd > 0) {
    //Write a string to it
    smbcw_fwrite(fd, content, strlen(content));

    //Close the file
    smbcw_fclose(fd);
  } else {
    perror("");
    printf("File could not be opened for write\n");
  }

  //Open the file again
  fd = smbcw_fopen(TEST_URL, "r");
  if (fd > 0) {
    char *buf = malloc(512);
    int count = 0;
    do {
      memset(buf, 0, 512);
      count = smbcw_fread(fd, buf, 512);
      printf("%s", buf);
    } while (count == 512);
    free(buf);
    smbcw_fclose(fd);
  } else {
    perror("");
    printf("File could not be opened for read\n");
  }

  //Read an directory and display the stats for each entry
  fd = smbcw_opendir(DIR_URL);
  if (fd > 0) {
    char *name;
    while ((smbcw_readdir(fd, &name) == 0) && name) {
      //Assemble a new URL for each file found
      int cnt = strlen(DIR_URL) + strlen(name) + 1;
      char *furl = malloc(cnt);
      memset(furl, 0, cnt);
      strcat(furl, DIR_URL);
      strcat(furl, name);

      //Get the stat for each file
      smbcw_stat stat;
      smbcw_urlstat(furl, &stat);

      //Print UID, GID, Mode, Size
      format_mode(stat.s_mode);
      printf("\t %d \t %d \t %ld \t", stat.s_uid, stat.s_gid, stat.s_size);

      //Print the name of each file
      printf("%s \n", name);

      free(furl);
    }

    smbcw_closedir(fd);
  }
}

