#include <mpi.h>

int main(int argc, char** argv)
{
    MPI_Init(NULL, NULL);

    int world_size = 2;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Finalize();
}