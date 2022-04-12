#include <stdio.h>
#include "fileio.h"

int main(int argc, char *argv[])
{
    File f1, f2, f3, f4;
    f1 = open_file("zza.dat");
    fs_print_error();
    f2 = open_file("zzb.dat");
    fs_print_error();
    f3 = open_file("zzc.dat");
    fs_print_error();
    f4 = open_file("zzd.dat");
    fs_print_error();
    write_file_at(f1, "Z", 1, BEGINNING_OF_FILE, 1L);
    fs_print_error();
    f2 = open_file("two.dat");
    fs_print_error();
    write_file_at(f2, "AB", 2, BEGINNING_OF_FILE, 0L);
    fs_print_error();
    write_file_at(f3, "MAZS", 4, BEGINNING_OF_FILE, 0L);
    fs_print_error();
    write_file_at(f4, "NZAT", 4, BEGINNING_OF_FILE, 0L);
    fs_print_error();
    write_file_at(f1, "M", 1, BEGINNING_OF_FILE, 0L);
    fs_print_error();
    close_file(f1);
    fs_print_error();
    close_file(f2);
    fs_print_error();
    close_file(f3);
    fs_print_error();
    close_file(f4);
    fs_print_error();
    return 0;
}