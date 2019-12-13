#include <stdio.h>

int fibbonacci(int n, int n1, int n2) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1, 0, 0) + fibbonacci(n-2, 0, 0));
   }
}

int main(){
    printf("Fib7: %i", fibbonacci(7, 0, 0));
}