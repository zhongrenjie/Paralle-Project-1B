#include <stdio.h>
#include <sys/time.h>
#include <stdint.h>

long int knapSack(long int C, long int w[], long int v[], int n);
long int MFKnap(long int w[], long int v[], int i, long int wt);

uint64_t GetTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*(uint64_t)1000000 + tv.tv_usec;
}
int main()
{
	long int C;
	int n;
	int i;

	scanf("%ld", &C);
	scanf("%d", &n);

	long int v[n], w[n];

	for (i = 0; i < n; i++)
	{
		scanf("%ld %ld", &v[i], &w[i]);
	}


	uint64_t start = GetTimeStamp();
	long int ks = MFKnap(w, v, n, C);
	printf("knapsack occupancy %ld\n", ks);
	printf("Time: %llu us\n", (uint64_t) (GetTimeStamp() - start));

	return 0;
}

long int max(long int x, long int y)
{
	return (x > y) ? x : y;
}

long int MFKnap(long int w[], long int v[], int i, long int wt)
{
	if (i == 0 || wt == 0)
	{
		return 0;
	}
	else
	{
		if (w[i-1] > wt)
		{
			return MFKnap(w, v, i-1, wt);
		}
		else
		{
			return max(MFKnap(w, v, i-1, wt), v[i-1] + MFKnap(w, v, i-1, wt-w[i-1]));
		}
	}

