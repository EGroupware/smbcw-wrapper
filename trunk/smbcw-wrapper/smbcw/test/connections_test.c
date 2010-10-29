//Compile with:
//gcc -o connections_test connections_test.c ../connections.c ../url.c -lsmbclient -g

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libsmbclient.h>

#include "../connections.h"
#include "../url.h"
#include "../smbcw_common.h"

typedef SMBCCTX *lp_smbcctx;

static void smb_auth_fn(const char *server, const char *share,
             char *workgroup, int wgmaxlen, char *username, int unmaxlen,
             char *password, int pwmaxlen)
{
	if ((workgroup != NULL) && (wgmaxlen >= strlen(DEFAULT_WORKGROUP)))
		strcpy(workgroup, DEFAULT_WORKGROUP);
	if ((username != NULL) && (unmaxlen >= strlen(DEFAULT_USERNAME)))
		strcpy(username, DEFAULT_USERNAME);
	if ((password != NULL) && (pwmaxlen >= strlen(DEFAULT_PASSWORD)))
		strcpy(password, DEFAULT_PASSWORD);
}

int main()
{
  lp_smbcw_url url;
  lp_smbcctx ctx;
  smbc_open_fn open_fn;
  smbc_close_fn close_fn;
  SMBCFILE *file;
  char *filename;

  //Initialize the connections system
  connections_init();

  //Initialize smbc
  smbc_init(smb_auth_fn, 0);

  //Get an URL descriptor for the following url
  url = smbcw_url_create("smb://andreas:pwd@localhost/daten/smbcw_is_watching_you.txt");

  //Get an SMBC context for that URL
  ctx = connections_get_ctx(url);

  //Get the create file function for the ctx
  open_fn = smbc_getFunctionOpen(ctx);

  //Get the url string
  filename = smbcw_url_gen_filename(url);

  file = open_fn(ctx, filename, O_CREAT, 0);
  if (!file) {
    perror("");
  }

  smbcw_url_free(url);
  free(filename);

  //Close the file again
  close_fn = smbc_getFunctionClose(ctx);
  close_fn(ctx, file);

  //Finalize the connections system
  connections_finalize();
}
