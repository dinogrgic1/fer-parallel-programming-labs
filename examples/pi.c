#include "mpi.h"
#include <stdio.h>
#include <math.h>

#define MASTER	0	// koji proces je voditelj (on ne racuna, samo rasporedjuje poslove)
#define JOBS_PER_PROC	100	// koliko poslova po procesu (prosjecno)

double dosinglejob(int start, int step, int n)
{
    double h, sum, x, mypi;
    int i;
    h   = 1.0 / (double) n;
    sum = 0.0;
    for (i = start; i <= n; i += step)
    {	x = h * ((double)i - 0.5);
        sum += 4.0 / (1.0 + x*x);
    }
    mypi = h * sum;
    return mypi;
}

int main(int argc, char *argv[])
{
    int done = 0, n, myid, numprocs, i;
    double PI25DT = 3.141592653589793238462643;
    double mypi, pi;
    double startwtime, endwtime;
    int  namelen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int jobs, start, jobsdone, procid, jobsgiven;
    MPI_Status status;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);

    printf("Proces %d na racunalu %s\n", myid, processor_name); fflush(stdout);

    n = 0;
    jobs = (numprocs - 1) * JOBS_PER_PROC;
    MPI_Barrier(MPI_COMM_WORLD);
    while (1)
    {	if (myid == MASTER)
        {	printf("Upisite broj elemenata reda: (0 za kraj) "); fflush(stdout);
            scanf("%d",&n);
            startwtime = MPI_Wtime();
        }

        MPI_Bcast(&n, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
        if (n == 0)	// kraj programa
            break;

        mypi = 0;
        if(myid == MASTER)	// master
        {	jobsdone = 0;
            while(jobsdone < jobs)
            {	// primimo zahtjev
                MPI_Recv(&procid, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                jobsdone++;
                // posaljemo posao (zapravo pocetnu vrijednost za petlju racunanja)
                MPI_Send(&jobsdone, 1, MPI_INT, procid, 99, MPI_COMM_WORLD);
            }
            // nema vise poslova! treba reci svima
            jobsdone = -1;
            for(i = 0; i<numprocs-1; i++)
            {	MPI_Recv(&procid, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                MPI_Send(&jobsdone, 1, MPI_INT, procid, 99, MPI_COMM_WORLD);
            }
        }
        else	// workers
        {	jobsgiven = 0;
            while(1)
            {	// trazimo posao
                MPI_Send(&myid, 1, MPI_INT, MASTER, 99, MPI_COMM_WORLD);
                // dobivamo podatak
                MPI_Recv(&start, 1, MPI_INT, MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if(start < 0)
                    break;	// nema vise poslova
                jobsgiven++;
                mypi += dosinglejob(start, jobs, n);
            }
            printf("Proces %d napravio %d (%.3g%%) poslova! (%s\b)\n",
                   myid, jobsgiven, 100*(double)(jobsgiven)/jobs, processor_name);
            fflush(stdout);
        }

        // prikupimo rezultate od svih (ukljucujuci i MASTER-a za koga je mypi = 0)
        MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        if (myid == MASTER)
        {	printf("\nPi je otprilike %.16f, pogreska je %.16f\n", pi, fabs(pi - PI25DT));
            endwtime = MPI_Wtime();
            printf("Proteklo vrijeme = %f\n\n", endwtime-startwtime);
        }
    }
    MPI_Finalize();
    return 0;
}