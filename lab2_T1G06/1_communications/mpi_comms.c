#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <mpi.h>

#define NUM_TEST 50
#define WAITING_TIME 1e6

void test_synchronous(int rank, int n);
void test_asynchronous(int rank, int n);

int main(int argc, char** argv)
{
    int rank;
    int size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size <= 2) {
        if (rank == 0)
            printf("Error, this program needs to be executed with more than 2 processors.\n");
        return 1;
    }

    /// FIRST TEST - SYNCHRONOUS COMMUNICATION
    test_synchronous(rank, size);

    /// FIRST TEST - ASYNCHRONOUS COMMUNICATION
    test_asynchronous(rank, size);

    MPI_Finalize();
    return 0;
}

void test_synchronous(int rank, int size)
{
    double start_time, run_time, average_time, max_time, min_time;
    int sbuf[NUM_TEST];
    int rbuf[NUM_TEST];
    MPI_Status status;

    /// TODO compute send/receive cores
    int send_core = (rank + 1) % size; //to which core do we send
    int recv_core = (rank + size - 1) % size; //who sends
    /// TODO

    start_time = MPI_Wtime();
    for(int i = 0; i < NUM_TEST; i++) {
        sbuf[i] = rank;

        /// TODO send
        MPI_Send(&sbuf[i], 1, MPI_INT, send_core, 0, MPI_COMM_WORLD);
        /// TODO

        if (i % size == rank)
            usleep(WAITING_TIME);

        /// TODO recv
        MPI_Recv(&rbuf[i], 1, MPI_INT, recv_core, 0, MPI_COMM_WORLD, &status);
        /// TODO

        assert(rbuf[i] == recv_core);
    }
    run_time = MPI_Wtime() - start_time;  

    /// TODO compute average, max and min times
    MPI_Reduce(&run_time, &average_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&run_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&run_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    /// TODO compute average, max and min times

    if (rank == 0) {
        average_time /= size;
        printf("Synchronous send/receive test with %d processes and %d repetitions.\n", size, NUM_TEST);
        printf("    average: %.2lf s\n", average_time);
        printf("    min:     %.2lf s\n", min_time);
        printf("    max:     %.2lf s\n", max_time);
    }

    MPI_Barrier(MPI_COMM_WORLD);

}

void test_asynchronous(int rank, int size)
{
    double start_time, run_time, average_time, max_time, min_time;
    int sbuf[NUM_TEST];
    int rbuf[NUM_TEST];
    /// TODO compute send/receive cores
    int send_core = (rank + 1) % size;
    int recv_core = (rank + size - 1) % size;
    /// TODO
    MPI_Request send_req[NUM_TEST], recv_req[NUM_TEST];
    MPI_Status send_status, recv_status;

    start_time = MPI_Wtime();

    for(int i = 0; i < NUM_TEST; i++) {
        sbuf[i] = rank;

        /// TODO async send
        MPI_Isend(&sbuf[i], 1, MPI_INT, send_core, 0, MPI_COMM_WORLD, &send_req[i]);
        /// TODO
       


        if (rank == i)
            usleep(WAITING_TIME);

        /// TODO async recieve
        MPI_Irecv(&rbuf[i], 1, MPI_INT, recv_core, 0, MPI_COMM_WORLD, &recv_req[i]);
        /// TODO
    }   
    /// TODO wait for async send/receive
    MPI_Waitall(NUM_TEST, send_req, MPI_STATUSES_IGNORE);
    MPI_Waitall(NUM_TEST, recv_req, MPI_STATUSES_IGNORE);
    /// TODO

    run_time = MPI_Wtime() - start_time;

    for(int i = 0; i < NUM_TEST; i++) {
        assert(rbuf[i] == recv_core);
    }

    /// TODO compute average, max and min times
    MPI_Reduce(&run_time, &average_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&run_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&run_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    /// TODO

    if (rank == 0) {
        average_time/=size;
        printf("Asynchronous send/receive test with %d processes and %d repetitions.\n", size, NUM_TEST);
        printf("    average: %.2lf s\n", average_time);
        printf("    min:     %.2lf s\n", min_time);
        printf("    max:     %.2lf s\n", max_time);
    }

}
