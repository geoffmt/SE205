#include <stdio.h>
#include <pthread.h>

void * myThread(void * parameter){
  printf("Hello World!\n%ld\n",pthread_self());
  return NULL;
}

int main(){
  pthread_t th;
  pthread_create(&th, NULL, myThread, NULL);
  pthread_join(th, NULL);
  printf("Hello main !\n");
  return 0;

}
