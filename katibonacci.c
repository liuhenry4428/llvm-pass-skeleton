#include <stdio.h>

int henribonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   }else if (n==2){
      return 2;
   } 
   else {
      return (henribonacci(n-1) + henribonacci(n-3));
   }
}

int main(){
    printf("Fib7: %i", henribonacci(40));
}