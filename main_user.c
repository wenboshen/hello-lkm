#include <stdio.h>

long long get_cycles(){
    unsigned int a=0, d=0;
    int ecx=(1<<30)+1; //What counter it selects?
    __asm __volatile("rdpmc" : "=a"(a), "=d"(d) : "c"(ecx));
    return ((long long)a) | (((long long)d) << 32);
}

int main (int argc, char* argv[])
{
    long long start, finish;
    int i;

    start = get_cycles();
    for (i = 0; i < 1000; i++)  {
        ;
    }
    finish = get_cycles();
    printf("Total cycles : %ld\n",((double)(finish-start))/1000.0);
    return 0;
}
