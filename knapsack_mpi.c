/* Knapsack calculation based on that of */
/* https://www.tutorialspoint.com/cplusplus-program-to-solve-knapsack-problem-using-dynamic-programming */

#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>
#include <mpi.h>
#include <math.h>
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



/* (No longer from the URL given in line 2) */

/*
    Sequential version which calculate the weight with preset
*/


/* PLACE YOUR CHANGES BELOW HERE */
#include <strings.h>
void print_arr(long int arr[], int n)
{
    printf("=====================================\n");
    for (int i = 0; i < n; i++)
    {
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
        cur_weight += used[i] * w[i];
    }

    if (cur_weight > C)
    {
        return 0;
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

void master(long int C, long int w[], long int v[], int n, long int * max)
{

    /*
        Message format:
            [prefix_bits, task_prefix]
            
    */

    const int SIG_TERM = -2;
    const int SIG_END = -1;

    //printf("Starting master........\n");
    int rank;
    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status s;
    int prefix_val = 0;
    int finished = 0;

    int loc=0; // Buffer for broadcast
    int loc_buf[size];
    long int tmp_res;

    const int prefix_bit = min(log2(size) + 1, n / 2);

    while (!finished)
    {
        if (prefix_val >= pow(2, prefix_bit))
        {
            for (int i = 0; i < size; i++)
            {
                loc_buf[i] = SIG_TERM;
            }
        }
        else
        {
            for (int i = 0; i < size; i++)
            {
                if (prefix_val < pow(2, prefix_bit))
                {
                    loc_buf[i]  = prefix_val;
                    prefix_val++;
                }
                else
                {
                    loc_buf[i]  = SIG_END;
                    prefix_val++;
                }
            }
        }

        MPI_Scatter(&loc_buf[0], 1, MPI_INT,&loc,1,MPI_INT, 0, MPI_COMM_WORLD);
        long int res;

        if (loc == SIG_END)
        {
            res = -1;
        }
        else if (loc == SIG_TERM)
        {
            //printf("TERM %d\n", rank);
            res = -1;
            finished = 1;
        }
        else
        {
            res = compute(C, w, v, n, prefix_bit, loc);
        }

        MPI_Reduce(&res, &tmp_res, 1, MPI_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
        if (rank == 0)
        {
            if (tmp_res > *max)
            {
                *max = tmp_res;
            }
        }

        //printf("Iterations: %d", prefix_val);
    }
    //printf("Rank %d Finished\n", rank);
}

long int knapSack(long int C, long int w[], long int v[], int N)
{
    int i;
    unsigned char used[N], solution[N];
    int done = 0;
    int rank;
    int size;
    long int max_value = 0;
    //int N = n;
    int n = N;
    long int c = C;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&c, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&w[0], N, MPI_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&v[0], N, MPI_LONG, 0, MPI_COMM_WORLD);
    N = n;
    C = c;
    if (size == 1)
    {
        return compute(C, w, v, N, 0, 0);
    }
    else
    {
        master(C, w, v, N, &max_value);
        //return compute(C, w, v, n, 0, 0);
    }

    

    int bin[3];

    dec2bin(&bin[0], 3, 3);

    bzero(used, sizeof(used));

    return max_value;
}

// mpicc knapsack_mpi.c -lm -fopenmp -o knapsack
