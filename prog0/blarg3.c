#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  unsigned long *p=malloc(sizeof(unsigned long));
  *p=0xdeadbeefcafeba00;
  int i;
  unsigned char *c = (unsigned char *)p;
  // output the individual bytes of the unsigned long value...or not?
  for (i=0; i < sizeof(unsigned long); i++) {
    printf("%02X ",*c);
    c++;
    
  }
  printf("\n");
  return 0;
}