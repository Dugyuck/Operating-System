#include <stdio.h>
#include "fileio.h"

int main(int argc, char *argv[]) {
  File f1, f2;
  f1=open_file("z1.dat");
  fs_print_error();
  f2=open_file("z2.dat");
  fs_print_error();
  write_file_at(f1, "MAZ", 3, BEGINNING_OF_FILE, 0L);
  fs_print_error();
  write_file_at(f2, "AZO", 3, BEGINNING_OF_FILE, 0L);
  fs_print_error();
  close_file(f1);
  fs_print_error();
  close_file(f2);
  fs_print_error();
  f1=open_file("z2.dat");
  fs_print_error();
  f2=open_file("z1.dat");
  write_file_at(f2, "Z", 1, BEGINNING_OF_FILE, 1L); //illegal
  fs_print_error();
  write_file_at(f1, "M", 1, BEGINNING_OF_FILE, 0L); //illegal
  fs_print_error();
  close_file(f1);
  fs_print_error();
  close_file(f2);
  fs_print_error();
  return 0;
}