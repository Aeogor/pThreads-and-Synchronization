#define _BSD_SOURCE

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>


// #if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
// /* union semun is defined by including <sys/sem.h> */
// #else
// /* according to X/OPEN we have to define it ourselves */
// union semun {
//     int val;                /* value for SETVAL */
//     struct semid_ds *buf;   /* buffer for IPC_STAT, IPC_SET */
//     unsigned short *array;  /* array for GETALL, SETALL */
//                             /* Linux specific part: */
//     struct seminfo *__buf;  /* buffer for IPC_INFO */
// };
// #endif

//ThreadInfo struct
typedef struct struct_Thread{
    int nBuffers;
    int workerID;
    double sleepTime;
    int semID;
    int mutexID;
    int *buffers;
    int nReadErrors;
    //Additional parameters may be necessary
    int lock;
}Thread;

/********************************************************************************************************/

//function to check if a number is prime or not
bool IsPrime(int number) {
    if (number <= 1)
        return false;

    int i;
    for (i=2; i*i<=number; i++) {
        if (number % i == 0) return false;
    }
    return true;
}

/********************************************************************************************************/

//Function to verify the arguments
bool checkArguments(int nBuffers, int nWorkers, double sleepMin, double sleepMax, int randSeed)
{
    if(nBuffers <= 2 || nBuffers >= 32) return false;
    if(!IsPrime(nBuffers)) return false;
    if(nWorkers < 1 || nWorkers >= nBuffers) return false;
    if(sleepMin < 0.0 || sleepMin >= sleepMax || sleepMax < 0.0) return false;
    if(randSeed < 0) return false;

    return true; //if reached here, all the arguments are fine
}

/********************************************************************************************************/

void wait(int id, int seg_num)
{
    struct sembuf s;
    s.sem_op = -1;
    s.sem_num = seg_num;
    s.sem_flg = SEM_UNDO;
    
    
    if(semop(id, &s, 1) == -1)
    {
        printf("ERROR getting semaphore\n");
        exit(-2);
    }
}

/********************************************************************************************************/

void signal(int id, int seg_num)
{
    struct sembuf s;
    s.sem_op = 1;
    s.sem_num = seg_num;
    s.sem_flg = SEM_UNDO;
    
    
    if(semop(id, &s, 1) == -1)
    {
        printf("ERROR getting semaphore\n");
        exit(-2);
    }
}

/********************************************************************************************************/

int qsortFunction (const void * a, const void * b)
{
        if (*(double*)a > *(double*)b) return -1;
        else if (*(double*)a < *(double*)b) return 1;
        else return 0;
}

/********************************************************************************************************/

void* worker(void* arg)
{
    //printf("LOCK\n");
    Thread* worker = (Thread*) arg;
    int start, current, next, write, i;
    int first_access;
    int second_access;
    bool lock = worker->lock;
    int count, currentCheck;
    
    start = worker->workerID;
    next = worker->workerID;

    for(i = 0; i < worker->nBuffers; ++i)
    {
        int j = 0;
        for(j = 0; j < 2; ++j)
        {
            //First read operation
            if(lock) wait(worker->semID, next);

            first_access = worker->buffers[next];
            usleep(worker->sleepTime);
            second_access = worker->buffers[next];

            if(lock) signal(worker->semID, next);

            if(first_access != second_access)
            {
                wait(worker->mutexID, 0);
                printf("Worker %d reported change in buffer %d from %d to %d. Bad Bits are ", worker->workerID, next, first_access, second_access);
                
                currentCheck = 0x01;
                count = 0;                
                while(currentCheck > 0)
                {
                    if((currentCheck & first_access) != (currentCheck & second_access))
                    {
                        if((currentCheck & first_access) > (currentCheck & second_access))
                            printf("-%d ", count);
                        else   
                            printf("%d ", count);
                    }
                    
                    currentCheck = currentCheck << 1;
                    ++count;
                }
                printf("\n");
                (worker->nReadErrors)++;

                signal(worker->mutexID, 0);
            }
            
            next = (next + start) % worker->nBuffers;
        }

        //Write operation
        if(lock) wait(worker->semID, next);

        first_access = worker->buffers[next];
        usleep(worker->sleepTime);
        worker->buffers[next] = first_access + (0x01 << (start - 1));

        if(lock) signal(worker->semID, next);
        
        next = (next + start) % worker->nBuffers;
    }

    pthread_exit(NULL);


}

/********************************************************************************************************/

int main(int argc, char ** argv)
{
    //printing the name
    printf("\nSrinivas Lingutla, slingu2, 655115444");
    printf(": Project 5 - Semaphores\n");
    printf("---------------------------------------\n\n");

    //Declare the variables
    int nBuffers;
    int nWorkers;
    double sleepMin = 1.0;
    double sleepMax = 5.0;
    int randSeed = 0;
    bool lock = false;
    int semID = -1;
    int mutexID;
    int totalReadErrors = 0;
    int totalWriteErrors = 0;
    int i;

    //parse the arguments
    if(argc < 3)
    {
        printf("Please run again with the required number of arguments\n");
        printf("./raceTest nBuffers nWorkers [ sleepMin sleepMax ] [ randSeed ] [ -lock | -nolock ]\n");
        exit(-1);
    }

    nBuffers = atoi(argv[1]);
    nWorkers = atoi(argv[2]);

    if(argc >= 4) //get sleepMin
    {
        sleepMin = atof(argv[3]);
    }

    if(argc >= 5) //get sleepMax
    {
        sleepMax = atof(argv[4]);
    }

    if(argc >= 6) //get randSeed
    {
        randSeed = atoi(argv[5]);
    }

    if(argc >=7 ) //get lock
    {
        if(strcmp("-lock", argv[6]) == 0) lock = true;
    }

    //Check the arguments
    if(!checkArguments(nBuffers, nWorkers, sleepMin, sleepMax, randSeed))
    {
        printf("Invalid arguments\n");
        exit(-1);
    }
    printf("nBuffers: %d\nnWorkers: %d\nsleepMin: %f\nsleepMax: %f\nrandSeed: %d\nLock: %d\n\n", nBuffers, nWorkers, sleepMin, sleepMax, randSeed, lock);

    //set the random seed for the program
    if(randSeed == 0) srand(time(NULL));
    else srand(randSeed);

    //create a local array of buffers
    int* buffers = (int*)malloc(sizeof(int) * nBuffers);
    for(i = 0; i < nBuffers; ++i)
    {
        buffers[i] = 0;
    }

    //created the mutexID
    mutexID = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if(mutexID == -1)
    {
        printf("Error creating semaphores!");
        exit(-2);
    }

    if(semctl(mutexID, 0, SETVAL, 1) == -1)
    {
        printf("failed to intialize mutex id\n");
    }

    //create the semID if neccessary
    if(lock == true)
    {
        semID = semget(IPC_PRIVATE, nBuffers, IPC_CREAT | 0600);
        if(semID == -1)
        {
            printf("Error creating semaphores!");
            exit(-2);
        }
        int n;
        for(n = 0; n < nBuffers; ++n) //initialize the semID's
        {
            if(semctl(semID, n, SETVAL, 1) == -1)
            {
                printf("failed to start sem id\n");
            }
        }
    }
    //sort the sleep times
    double sortedSleepTimes[nWorkers];
    
    for(i = 0; i < nWorkers; ++i)
    {
        sortedSleepTimes[i] = (sleepMin + (sleepMax - sleepMin)*rand()/RAND_MAX);
    }
    
    qsort((void*)sortedSleepTimes, nWorkers, sizeof(sortedSleepTimes[0]), qsortFunction);

    //create nWorkers structs
    Thread* threads = (Thread*)malloc(sizeof(struct struct_Thread) * nWorkers);

    //populate each of the threads
    for(int i = 0; i < nWorkers; ++i)
    {
        threads[i].nBuffers = nBuffers;
        threads[i].workerID = i + 1;
        threads[i].sleepTime = sortedSleepTimes[i];
        threads[i].semID = semID;
        threads[i].mutexID = mutexID;
        threads[i].buffers = buffers;
        threads[i].nReadErrors = 0;
        threads[i].lock = lock;

        printf("sleeptime[%d] = %lf\n", i, threads[i].sleepTime);
        threads[i].sleepTime *= 1000000;
    }
    
    printf("\n");

    //Start the pthreads
    pthread_t pthreads[nWorkers];

    for(i = 0; i < nWorkers; ++i)
    {
        pthread_create(&pthreads[i], NULL, worker, (void*) &threads[i]);
    }

    for(i = 0; i < nWorkers; ++i)
    {
        pthread_join(pthreads[i], NULL);
    }
    //End of the pthreads
    
    int expectedValue = ( 1 << nWorkers ) - 1;
    
    printf("\nAll of the buffers should hold %d\n\n", expectedValue);
    
    for(i = 0; i < nBuffers; ++i)
    {
        printf("Buffer %d holds %d", i , buffers[i]);
        if(buffers[i] != expectedValue)
        {
            printf(", Bad bits: %d", i);
            int currentCheck = 0x01;
            int count = 0;                
            while(currentCheck > 0)
            {
                if((currentCheck & expectedValue) != (currentCheck & buffers[i]))
                {
                    printf("%d ", count);
                    totalWriteErrors+=1;
                }
                
                currentCheck = currentCheck << 1;
                ++count;
            }
        }
        printf("\n");
    }
    
    //Get read errors
    for(i = 0 ; i < nBuffers; ++i)
    {
        totalReadErrors += threads[i].nReadErrors;
    }
    printf("Total Read Errors: %d\n", totalReadErrors);
    printf("Total Write Errors: %d\n", totalWriteErrors);


    //remove the semaphores
    if(semctl(mutexID, 0, IPC_RMID) == -1)
    {
        printf("failed to remove mutex id\n");
    }
    if(lock)
        if(semctl(semID, nBuffers, IPC_RMID) == -1)
            printf("failed to remove mutex id\n");
    return 0;
}

/********************************************************************************************************/