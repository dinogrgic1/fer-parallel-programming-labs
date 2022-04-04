#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define MAX_SLEEP_TIME 5
#define MPI_L_FORK_REQUEST 10
#define MPI_R_FORK_REQUEST 20
#define MPI_L_FORK_SEND 30
#define MPI_R_FORK_SEND 40
#define MPI_FINISHED 50

#define FORK_CLEAN 0
#define FORK_DIRTY 1
#define FORK_MISSING 2

void sleep_multi_os(unsigned long time)
{
    #ifdef _WIN32
    Sleep(time);
    #else
    sleep(time);
    #endif
}

unsigned int_rand(int min, int max)
{
    return (rand() % max) + min;
}

int get_next(int rank, int N)
{
    return (rank + 1 >= N) ? 0 : rank + 1;
}

int get_previous(int rank, int N)
{
    return (rank - 1 < 0) ? N - 1 : rank - 1;
}

void respond(int rank, int N, int* l_spoon, int* r_spoon, int* l_request, int* r_request, int wait)
{
    int flag;
    MPI_Status status;

    MPI_Request l_status;
    MPI_Request r_status;
    MPI_Request l_requested = MPI_SUCCESS;
    MPI_Request r_requested = MPI_SUCCESS;

    MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    while(!flag && wait)
    {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    }

    if(status.MPI_TAG == MPI_R_FORK_REQUEST)
    {
        MPI_Recv(r_request, 1, MPI_INT, get_previous(rank, N), MPI_R_FORK_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (*l_spoon == FORK_DIRTY)
        {
            *l_spoon = FORK_CLEAN;
            MPI_Send(l_spoon, 1, MPI_INT, get_previous(rank, N), MPI_L_FORK_SEND, MPI_COMM_WORLD);
            *l_spoon = FORK_MISSING;
        }
    }

    if(status.MPI_TAG == MPI_L_FORK_REQUEST)
    {
        MPI_Recv(l_request, 1, MPI_INT, get_next(rank, N), MPI_L_FORK_REQUEST, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (*r_spoon == FORK_DIRTY)
        {
            *r_spoon = FORK_CLEAN;
            MPI_Send(r_spoon, 1, MPI_INT, get_next(rank, N), MPI_R_FORK_SEND, MPI_COMM_WORLD);
            *r_spoon = FORK_MISSING;
        }
    }

    if(status.MPI_TAG == MPI_L_FORK_SEND)
    {
        MPI_Recv(r_spoon, 1, MPI_INT, get_next(rank, N), MPI_L_FORK_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    if(status.MPI_TAG == MPI_R_FORK_SEND)
    {
        MPI_Recv(l_spoon, 1, MPI_INT, get_previous(rank, N), MPI_R_FORK_SEND, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void think(int rank, int N, int* l_spoon, int* r_spoon, int* l_request, int* r_request)
{
    unsigned random_num = int_rand(1, MAX_SLEEP_TIME);
    printf("%*s\n", rank + 6, "mislim");
    fflush(stdout);

    for(int i = 0; i < random_num; i++)
    {
        sleep_multi_os(random_num * 1000);
        respond(rank, N, l_spoon, r_spoon, l_request, r_request, 0);
    }

}

void hungry(int rank, int N, int* l_spoon, int* r_spoon, int* l_request, int* r_request)
{
    while(*l_spoon == FORK_MISSING || *r_spoon == FORK_MISSING)
    {
        MPI_Request l_requested = MPI_SUCCESS;
        MPI_Request r_requested = MPI_SUCCESS;

        if (*l_spoon == FORK_MISSING)
        {
            printf("%*strazim(%d)\n", rank, "", get_previous(rank, N));
            fflush(stdout);
            MPI_Send(&rank, 1, MPI_INT, get_previous(rank, N), MPI_L_FORK_REQUEST, MPI_COMM_WORLD);
        }

        if (*r_spoon == FORK_MISSING)
        {
            printf("%*strazim(%d)\n", rank, "", get_next(rank, N));
            fflush(stdout);
            MPI_Send(&rank, 1, MPI_INT, get_next(rank, N), MPI_R_FORK_REQUEST, MPI_COMM_WORLD);
        }

        respond(rank, N, l_spoon, r_spoon, l_request, r_request, 1);
    }
}

void eat(int rank, int N, int* l_spoon, int* r_spoon, int* l_request, int* r_request)
{
    printf("%*s\n", rank + 5, "jedem");
    fflush(stdout);

    *l_spoon = FORK_DIRTY;
    *r_spoon = FORK_DIRTY;

    respond(rank, N, l_spoon, r_spoon, l_request, r_request, 1);
    MPI_Barrier(MPI_COMM_WORLD);
    respond(rank, N, l_spoon, r_spoon, l_request, r_request, 0);
}

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size <= 1)
    {
        fprintf(stderr, "[ERROR] Broj filozofa mora biti n > 1");
        MPI_Finalize();
        return -1;
    }

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int l_spoon = FORK_MISSING;
    int r_spoon = FORK_DIRTY;

    int l_request = -1;
    int r_request = -1;

    if (world_rank == 0) {
        l_spoon = FORK_DIRTY;
    }

    if (world_rank == world_size - 1) {
        r_spoon = FORK_MISSING;
    }

    srand(time(NULL) + world_rank);

    think(world_rank, world_size, &l_spoon, &r_spoon, &l_request, &r_request);
    hungry(world_rank, world_size, &l_spoon, &r_spoon, &l_request, &r_request);
    eat(world_rank, world_size, &l_spoon, &r_spoon, &l_request, &r_request);

    MPI_Finalize();
}

/*
Proces(i)
        {	misli (slucajan broj sekundi);					// ispis: mislim
        i 'istovremeno' odgovaraj na zahtjeve!			// asinkrono, s povremenom provjerom (vidi nastavak)
        dok (nemam obje vilice) {
            posalji zahtjev za vilicom;				// ispis: trazim vilicu (i)
            ponavljaj {
                cekaj poruku (bilo koju!);
                ako je poruka odgovor na zahtjev		// dobio vilicu
                azuriraj vilice;
                inace ako je poruka zahtjev			// drugi traze moju vilicu
                obradi zahtjev (odobri ili zabiljezi);
            } dok ne dobijes trazenu vilicu;
        }
        jedi;								// ispis: jedem
        odgovori na postojeće zahtjeve;					// ako ih je bilo
        }*/
