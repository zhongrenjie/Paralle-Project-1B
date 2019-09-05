#include <stdio.h>
#include <string.h>

long int compute(long int C, long int w[], long int v[], int n, int len, int loc)
{
    unsigned char used[n], solution[n];
    long int cur_value = 0;
    long int cur_weight = 0;
    long int max_value = 0;
    long int weight_of_max = 0;
    int done = 0;
    int i;

    bzero (used, sizeof (used));

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
        return -1;
    }

    while (!done) {
        done = 1; // 用来标识计算是否结束
        for (i = len; i < n; i++)
        {
            if (!used[i]) {       // 如果第i个物品没有被使用，那么使用第i个物品
                used[i] = 1;      // 将used数组中的值标识为1
                cur_weight += w[i];   // 加上第i个物品的重量
                cur_value += v[i];    // 加上第i个物品的价值
                done = 0;          // 将done赋值为0，这样就会继续while循环
                break;
            } else {
                used[i] = 0;
                cur_weight -= w[i];
                cur_value -= v[i];
            }
        }
        if (cur_weight <= C && cur_value > max_value) {
            max_value = cur_value;
            weight_of_max = cur_weight;
            bcopy (used, solution, sizeof (used));
        }
    }
    return max_value;

}

int main()
{
    long int C;    /* capacity of backpack */
    int n;    /* number of items */
    int i;    /* loop counter */
    long int result = 0;


    scanf ("%ld", &C);
    scanf ("%d", &n);
    long int v[n], w[n];        /* value, weight */

    for (i = 0; i < n; i++) {
        scanf ("%ld %ld", &v[i], &w[i]);
    }


    result = compute(C, w, v, n, 3, 3);

    printf("The reuslt is: %ld\n", result);


    return 0;
}