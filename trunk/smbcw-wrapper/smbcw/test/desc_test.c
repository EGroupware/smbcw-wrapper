
//Compile with gcc -o desc_test desc_test.c ../descriptor.c

#include <stdio.h>
#include <stdlib.h>

#include "../descriptor.h"

void test_id(void *ptr)
{
  int id = smbcw_gen_id(ptr);

  printf("Generated ID %d for ptr %p\n", id, ptr);
//  smbcw_free_id(id);
}

int main()
{
  //Test generating some IDs
  test_id((void*)0xFFFFFFFF);
  test_id((void*)0xFFFFFFFF);
  test_id((void*)0xFFFFFFFF);
  smbcw_free_id(2);
  test_id((void*)0xFFFFFFF0);
  smbcw_free_id(1);
  smbcw_free_id(0);
  printf("ptr of id 2: %p\n", smbcw_get_ptr(2));
  smbcw_free_id(2);
}

