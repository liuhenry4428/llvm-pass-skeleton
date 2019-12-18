#include <stdio.h>

int fibbonacci(int n) {
   if(n==0) return 0;
   if(n==1) return 1;
   int i1 = 0;
   int i2 = 1;
   int i = 1;
   while(i < n){
      i++;
      int current = i1+i2;
      i1 = i2;
      i2 = current;
   }
   return i2;
}

int main(){
    printf("Fib7: %i", fibbonacci(3));
}