//Compile with gcc ../url.c url_test.c -g -o url_test

#include <stdio.h>

#include "../url.h"

void dump_url(lp_smbcw_url url)
{
	printf("Protocol: %s\n", url->protocol);
	printf("User: %s\n", url->user);
	printf("Password: %s\n", url->password);
	printf("Host: %s\n", url->host);
	printf("Port: %s\n", url->port);
	printf("Path: %s\n", url->path);
}

void test_url(const char *url)
{
	printf("\nTesting %s\n", url);
	lp_smbcw_url _url = smbcw_url_create(url);
	dump_url(_url);
	smbcw_url_free(_url);
}

int main() {
	test_url("/test/andreas/spezifikationen.c");
	test_url("test/andreas/spezifikationen.c");
	test_url("http://test/andreas/spezifikationen.c");
	test_url("http://test:3000/andreas/spezifikationen.c");
	test_url("smb://host:port");
	test_url("http://foo@bar:1000");
	test_url("file://127.0.0.1/@bla");
	test_url("smb://user:pwd@host.com:1080/path/to/some/file.txt");
}

