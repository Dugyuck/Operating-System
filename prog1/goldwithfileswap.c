#include <stdio.h>
#include "fileio.h"

int main(int argc, char *argv[]) {
  File f,f2;

  f=open_file("anotherdat.dat");
  fs_print_error();
  write_file_at(f, "BEWEAROFMZ", strlen("BEWEAROFMZ"), BEGINNING_OF_FILE, 0L);
  fs_print_error();
  close_file(f);
  fs_print_error();
  f2=open_file("anotherdat.dat");
  fs_print_error();
  write_file_at(f2, "Z", 1, END_OF_FILE, -9L);
  fs_print_error();
   close_file(f2);
  fs_print_error();
  f=open_file("anotherdat.dat");
  fs_print_error();
  write_file_at(f, "M", 1, END_OF_FILE, -10L);
  fs_print_error();
  close_file(f);
  fs_print_error();
  return 0;
}