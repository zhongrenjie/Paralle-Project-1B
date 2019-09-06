/* Knapsack calculation based on that of */
/* https://www.tutorialspoint.com/cplusplus-program-to-solve-knapsack-problem-using-dynamic-programming */

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <mpi.h>
#include <math.h>
#include <omp.h>
#include <unistd.h>

long int knapSack(long int C, long int w[], long int v[], int n);

uint64_t GetTimeStamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

int main(int argc, char *argv[])
{
    long int C; /* capacity of backpack */
    int n;      /* number of items */
    int i;      /* loop counter */

    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        scanf("%ld", &C);
        scanf("%d", &n);
    }

    long int v[n], w[n]; /* value, weight */

    if (rank == 0)
    {
        for (i = 0; i < n; i++)
        {
            scanf("%ld %ld", &v[i], &w[i]);
        }
    }

    uint64_t start = GetTimeStamp();
    long int ks = knapSack(C, w, v, n);

    if (rank == 0)
    {
        printf("knapsack occupancy %ld\n", ks);
        printf("Time: %ld us\n", (uint64_t)(GetTimeStamp() - start));
    }

    MPI_Finalize();

    return 0;
}

/* PLACE YOUR CHANGES BELOW HERE */
#include <strings.h>

long int max(long int x, long int y)
{
    return (x > y) ? x : y;
}

/* (No longer from the URL given in line 2) */

/*
    Sequential version which calculate the weight with preset
*/

void print_arr(long int arr[], int n) {
    printf("=====================================\n");
    for(int i=0;i<n;i++) {
        printf("%ld ", arr[i]);
    }

    printf("\n");
            printf("=====================================\n");

}


void dec2bin(int *bin_arr, int dec, int n_bits)
{
    for (int i = 0; i < n_bits; i++)
    {
        bin_arr[n_bits - i - 1] = dec % 2;
        dec /= 2;
    }
}

int min(int a, int b)
{
    return a < b ? a : b;
}

long int compute(long int C, long int w[], long int v[], int n, int len, int loc)
{
    char used[n], solution[n];
    long int cur_value = 0;
    long int cur_weight = 0;
    long int max_value = 0;
    long int weight_of_max = 0;
    int done = 0;
    int i;

    bzero(used, sizeof(used));

    for (i = 0; i < len; i++)
    {
        used[len - 1 - i] = loc % 2;
        loc /= 2;
    }

    for (i = 0; i < len; i++)
    {
        cur_value += used[i] * v[i];
        printf("used: %d, w: %ld\n", used[i], w[i]);
        cur_weight += used[i] * w[i];
    }

    if (cur_weight > C)
    {
        printf("cur_weight: %ld\n",cur_weight);
        return -1;
    }

    while (!done)
    {
        done = 1;
        for (i = len; i < n; i++)
        {
            if (!used[i])
            {
                used[i] = 1;
                cur_weight += w[i];
                cur_value += v[i];
                done = 0;
                break;
            }
            else
            {
                used[i] = 0;
                cur_weight -= w[i];
                cur_value -= v[i];
            }
        }
        if (cur_weight <= C && cur_value > max_value)
        {
            max_value = cur_value;
            weight_of_max = cur_weight;
            bcopy(used, solution, sizeof(used));
        }
    }
    return max_value;
}



long int master(long int C, long int w[], long int v[], int n)
{

    /*
        Message format:
            [prefix_bits, task_prefix]
            
    */
    printf("Starting master........\n");
    int rank;
    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request rq[100], re[100];
    MPI_Status s;
    int recvd[100];
    const int prefix_bit = min(log2(size) + 1, n / 2);
    int msg_snd[2];
    long int msg_recv[1];

    int prefix_val = 0;
    long int max = 0;
    int finished = 0;

    int i; //循环体

    long int buf[2*n + 1]; // Buffer for broadcast
    buf[0] = n;
    MPI_Bcast(&buf[0], 1, MPI_INT, 0, MPI_COMM_WORLD);  // broadcast n

    buf[0] = C;
    for (i = 0; i < n; i++)
    {
        buf[i + 1] = w[i];
        buf[i + n + 1] = v[i];
    }

    print_arr(buf, 2*n + 1);

    MPI_Bcast(&buf[0], 2*n + 1, MPI_LONG, 0, MPI_COMM_WORLD);  // broadcast C, w and v


    printf("Prefix size: %d\n", prefix_bit);
    for (int i = 0; i < size; i++)
    {
        msg_snd[0] = prefix_bit;
        msg_snd[1] = prefix_val;
        MPI_Isend(&msg_snd[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &re[0]);
        MPI_Wait(&re[0], &s);
        printf("Error: %d\n", s.MPI_ERROR);
    }
    for (i = 1; i < size; i++)
    {
        MPI_Irecv(&msg_recv[0], 1, MPI_LONG, i, 2, MPI_COMM_WORLD, &rq[i]);
    }
    while (!finished)
    {
        for (i = 1; i < size; i++)
        {
            int err = MPI_Test(&rq[i], &recvd[i], MPI_STATUS_IGNORE);
            if (recvd[i])
            {
                printf("Reveived result from slave %d : %ld\n", i, msg_recv[0]);

                recvd[i] = 0;
                if (msg_recv[0] > max)
                {
                    max = msg_recv[0];
                }
                prefix_val++;
                int thres = pow(2, prefix_bit);
                if (prefix_val >= thres)
                {
                    finished = 1;
                }
                else
                {
                    msg_snd[0] = prefix_bit;
                    msg_snd[1] = prefix_val;
                    MPI_Isend(&msg_snd[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &re[0]);
                    MPI_Irecv(&msg_recv[0], 1, MPI_LONG, i, 2, MPI_COMM_WORLD, &rq[i]);
                }
            }
        }
    }

    for (int i = 1; i < size; i++)
    {
        msg_snd[0] = -1;
        msg_snd[1] = -1;
        MPI_Isend(&msg_snd[0], 2, MPI_INT, i, 1, MPI_COMM_WORLD, &re[i + size]);
        printf("Sent TERM sig to slave: %d\n", i);
    }
    printf("Result: %ld\n", max);
    return max;
}

long int slave()
{

    int rank;
    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("Starting slave %d.........\n", rank);

    long int msg_snd[1];
    int msg_recv[2];
    MPI_Request rq[1], re[1];
    MPI_Status s;
    int recved = 0;
    int finished = 0;
    int n = 0;

    /*接收Bcast*/
    int i;
    int x[1];
    MPI_Bcast(&x[0], 1, MPI_INT, 0, MPI_COMM_WORLD);
    // printf("%d\n", x[0]);
    n = x[0];
    long int buf[2*n + 1];
    long int w[n], v[n];
    long int C = 0;
    MPI_Bcast(&buf[0], 2*n+1, MPI_LONG, 0, MPI_COMM_WORLD);
    C = buf[0];
    for (i = 0; i < n; i++)
    {
        w[i] = buf[i + 1];
        v[i] = buf[i + n + 1];
    }
    print_arr(buf, 2*n + 1);

    while (!finished)
    {
        MPI_Irecv(&msg_recv[0], 2, MPI_INT, 0, 1, MPI_COMM_WORLD, &rq[0]);

        MPI_Wait(&rq[0], &s);
        printf("\rSlavePolling %d\n", s.MPI_ERROR);

        recved = 0;
        if (msg_recv[0] < 0)
        {
            printf("Slave %d terminated\n", rank);
            break;
        }

        printf("Received data from master: %d, %d\n", msg_recv[0], msg_recv[1]);


        long int res = compute(C, w, v, n, msg_recv[0], msg_recv[1]);

        msg_snd[0] = res;
        MPI_Isend(&msg_snd[0], 1, MPI_LONG, 0, 2, MPI_COMM_WORLD, &re[0]);
        MPI_Wait(&re[0], &s);
        /*
        while (!s.MPI_ERROR)
        {
            printf("R...");
            MPI_Isend(&msg_snd[0], 1, MPI_LONG_INT, 0, 2, MPI_COMM_WORLD, &re[0]);
        }
        */
    }
    return 0;
}

int C_;
long int *w_;
long int *v_;
int n_;

long int knapSack(long int C, long int w[], long int v[], int n)
{
    int i;
    unsigned char used[n], solution[n];
    int done = 0;
    int rank;
    int size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("%d\n", rank);

    if (size == 1)
    {
        return compute(C, w, v, n, 0, 0);
    }

    if (rank == 0)
    {
        //slave(C, w, v, n);
        master(C, w, v, n);
    }
    else
    {
        slave();
    }

    int bin[3];

    dec2bin(&bin[0], 3, 3);

    bzero(used, sizeof(used));
    long int max_value = 0;

    return max_value;
}

// mpicc knapsack_mpi.c -lm -fopenmp -o knapsack
