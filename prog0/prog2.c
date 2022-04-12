#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct funcs {
int (*openit)(char *name, int prot);
void (*closeit)(void);
} funcs;

int my_openit(char *name, int prot)
{
    printf("Name: %s\nNumber: %d\n", name, prot);
    return 0;
}

void my_closeit()
{
    printf("close");
}

void f(funcs *temp){
    temp->openit = my_openit;
    temp->closeit = my_closeit;
    
}
static funcs fields = 
{
    .openit = my_openit,
    .closeit = my_closeit
};
int main()
{
   struct funcs newFunc;
   f(&newFunc);
    f(&fields);
   
   
    return 0;
}