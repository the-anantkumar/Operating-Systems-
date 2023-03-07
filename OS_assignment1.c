#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<math.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<pthread.h>
#include<signal.h>
#include<CSF372.h>
 
int** arr;
pid_t* pid;
int** fd;
pthread_t* tid;
int n, a, b, p;
 
int is_prime(int n) {
        if(n < 2)
            return 0;
        if(n < 4)
            return 1;
        if(n % 2 == 0 || n % 3 == 0)
            return 0;
        for(int i = 5; i*i <= n; i += 6)
            if(n % i == 0 || n % (i+2) == 0)
                return 0;
        
        printf("prime number: %d discovered.\n", n);
        return 1;
}
 
void* slave(void* arg){
    int denominator=2*p;
    int id=(int)arg;
    int sumofprimes=0;
    //previous p
    int pcount=p;
    int num=id-1;
    while(pcount>0){
        if(is_prime(num)==1){
            sumofprimes+=num;
            pcount--;
        }
        num--;
    }
    //check id
    if(is_prime(id)==1){
        sumofprimes+=id;
        denominator++;
    }
    //next p
    pcount=p;
    num=id+1;
    while(pcount>0){
        if(is_prime(num)==1){
           
            sumofprimes+=num;
            pcount--;
        }
        num++;
    }
    PRINT_INFO("computed px of current element of the n * n matrix\n");
 
    int* result=(int*) malloc(sizeof(int));
    *result=sumofprimes/denominator;
    return (void*)result;
}
int main(int argc, char *argv[]){
 
    //read arguments
    n=atoi(argv[1]);
    a=atoi(argv[2]);
    b=atoi(argv[3]);
    p=atoi(argv[4]);
    printf("The value of n is %d, the value of a is %d, the value of b is %d and the value of p is %d",n,a,b,p);   //change english
 
    //check if number of arguments is correct
    printf("%d\n", argc);
    printf("%d\n",(n*n)+5);
    if(argc!=((n*n)+5)){
       PRINT_INFO("Illegal number of arguments\n");        //change error message
    }
    
 
    //allocate memory for array and store values
    int loc = 0, offset = 5;
    arr = (int**)malloc(n * sizeof(int*));
    for(int i=0; i<n; i++){
        arr[i]= (int*)malloc(n * sizeof(int));
        for(int j = 0; j < n; j++){
            arr[i][j]=atoi(argv[offset + loc]);
            loc++;
        }
    }
 
    //initialize pid and fd for children processes and pipes
    pid=(pid_t*)malloc(n*sizeof(pid_t));
    fd=(int**)malloc(n*sizeof(int*));
    for(int i=0; i<n; i++){
        fd[i]=(int*)malloc(2*sizeof(int));
    }
 
    //fork processes
    int idx; 
    for(idx=0; idx<n; idx++){
      PRINT_INFO("Creating pipe and the worker processes.\n");
        pipe(fd[idx]);
        pid[idx] = fork();
        if(pid[idx] == 0) break;
    }
 
 
    if(idx==n){
        //parent
        int sumofwpapx=0;
        for(int i=0; i<n; i++){
            int piperes;
           PRINT_INFO("Worker is executing and it is processing row : %d", i);
            waitpid(pid[i],NULL, NULL);
            close(fd[i][1]);
            read(fd[i][0], &piperes, sizeof(piperes));
            close(fd[i][0]);
            printf("wpapx:%d\n", piperes);
            sumofwpapx+=piperes;
        }
       PRINT_INFO("computing fapx..\n");
        int fapx=sumofwpapx/n;
        printf("fapx: %d",fapx);
        exit(0);
    }
 
 
 
    tid=(pthread_t*)malloc(n*sizeof(pthread_t));
    PRINT_INFO("Worker thread created and now Processing rows..\n");
    for(int col=0; col<n; col++){
        if(arr[idx][col]<a||arr[idx][col]>b){
            //code for invalid input

        }

        printf("%d\n", arr[idx][col]);
        pthread_create(&tid[col], NULL, slave, (void*)arr[idx][col]);
    }
    int rowsum=0;
    for(int col=0; col<n; col++){
        int*thapx;
        PRINT_INFO("Thread about to join and terminate.\n");
        pthread_join(tid[col], (void**)&thapx);
        printf("thapx of element[%d][%d]: %d\n",idx,col,*thapx);
        rowsum+=(*thapx);
    }
    printf("Computing wpapx..\n");
    int wpapx=rowsum/n;
    printf("wpapx of row%d: %d\n",idx,wpapx);
    close(fd[idx][0]);
    PRINT_INFO("Writing wpapx to the controller.\n");
    write(fd[idx][1], &wpapx, sizeof(wpapx));
    close(fd[idx][1]);
}