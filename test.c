#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// 定义物品结构体，包含重量、价值、编号以及单位价值（用于贪心法）
typedef struct {
    int weight;
    double value;
    int id;
    double ratio; // 单位价值（value / weight），用于贪心策略排序
} Item;

// 随机生成 n 个物品的数据
void generate_items(Item *items, int n) {
    for (int i = 0; i < n; i++) {
        items[i].id = i;
        items[i].weight = rand() % 100 + 1;             // 重量在 1~100 之间
        items[i].value = (rand() % 90001) / 100.0 + 100.0; // 价值在 100.00~1000.00 之间
    }
}

// 将物品数据保存为 CSV 文件
void item_data_save(Item *items, int n, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("打开文件时出错");
        return;
    }

    fprintf(fp, "id,weight,value\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "%d,%d,%.2f\n", items[i].id, items[i].weight, items[i].value);
    }
    fclose(fp);
}

// 蛮力法：枚举所有物品子集组合，找出总价值最大且不超过容量的解
double test_brute_force(int n, int C, Item *items, int *selected, int *visited_states) {
    double max_value = 0;  // 最大总价值
    *visited_states = 0;   // 状态访问次数计数器

    // 枚举所有子集（共 2^n 种可能）
    for (int subset = 0; subset < (1 << n); ++subset) {
        (*visited_states)++;
        double total_value = 0;  // 当前子集的总价值
        int total_weight = 0;    // 当前子集的总重量

        // 检查当前子集中哪些物品被选中
        for (int i = 0; i < n; ++i) {
            if ((subset >> i) & 1) {  // 如果第 i 位是 1，则表示选中该物品
                total_value += items[i].value;
                total_weight += items[i].weight;
            }
        }

        // 如果不超重，并且总价值更大，则更新最优解
        if (total_weight <= C && total_value > max_value) {
            max_value = total_value;
            memset(selected, 0, n * sizeof(int));
            for (int i = 0; i < n; ++i) {
                selected[i] = (subset >> i) & 1;  // 更新最佳选择方案
            }
        }
    }

    return max_value;
}

// 动态规划法：使用一维数组优化空间复杂度，记录每个容量下的最大价值
double test_dp(int n, int C, Item *items, int *selected, int *visited_states) {
    double *dp = calloc(C + 1, sizeof(double));               // dp[j] 表示容量为 j 时的最大价值
    int *trace = calloc((n + 1) * (C + 1), sizeof(int));       // trace[i][j] 记录是否选用了第 i 个物品
    *visited_states = 0;

    // 对于每个物品，从后往前更新 dp 数组
    for (int i = 0; i < n; ++i) {
        for (int j = C; j >= items[i].weight; --j) {
            (*visited_states)++;
            double option = dp[j - items[i].weight] + items[i].value;
            if (option > dp[j]) {
                dp[j] = option;
                trace[i * (C + 1) + j] = 1;  // 标记该物品被选中
            } else {
                trace[i * (C + 1) + j] = 0;  // 不选
            }
        }
    }

    double best_value = dp[C];  // 最终最大价值
    int cap = C;                // 回溯路径找选中的物品
    memset(selected, 0, n * sizeof(int));

    // 从后向前查找被选中的物品
    for (int i = n - 1; i >= 0; --i) {
        if (trace[i * (C + 1) + cap]) {
            selected[i] = 1;
            cap -= items[i].weight;
        }
    }

    free(dp);
    free(trace);
    return best_value;
}

// 用于 qsort 的比较函数，按单位价值降序排序
static int compare_items(const void *a, const void *b) {
    Item *itemA = (Item *)a;
    Item *itemB = (Item *)b;
    return (itemB->ratio > itemA->ratio) ? 1 : -1;
}

// 贪心法：优先选取单位价值高的物品，直到不能再装为止
double test_greedy(int n, int C, Item *items, int *selected, int *visited_states) {
    *visited_states = 0;

    // 创建物品副本以便排序，保留原始顺序
    Item *copy = malloc(n * sizeof(Item));
    memcpy(copy, items, n * sizeof(Item));

    // 计算每个物品的单位价值
    for (int i = 0; i < n; ++i) {
        copy[i].ratio = copy[i].value / copy[i].weight;
    }

    // 按照单位价值降序排序
    qsort(copy, n, sizeof(Item), compare_items);

    double total_value = 0;     // 总价值
    int current_weight = 0;     // 当前背包已装重量
    memset(selected, 0, n * sizeof(int));

    // 依次尝试装入物品
    for (int i = 0; i < n; ++i) {
        (*visited_states)++;
        if (current_weight + copy[i].weight <= C) {
            selected[copy[i].id] = 1;
            total_value += copy[i].value;
            current_weight += copy[i].weight;
        }
    }

    free(copy);
    return total_value;
}

// 回溯法递归核心函数：深度优先搜索所有物品选择路径
static void dfs_backtrack(int idx, int n, int capacity, Item *items,
                         int *cur_selection, double cur_value, int cur_weight,
                         double *best_value, int *best_selection, int *visited_states) {

    (*visited_states)++;
    // 边界条件：所有物品已处理完 或 已满
    if (idx == n || cur_weight == capacity) {
        if (cur_value > *best_value) {
            *best_value = cur_value;
            memcpy(best_selection, cur_selection, n * sizeof(int));
        }
        return;
    }

    // 不选当前物品
    dfs_backtrack(idx + 1, n, capacity, items, cur_selection, cur_value, cur_weight, best_value, best_selection, visited_states);

    // 如果还能装下当前物品，则尝试选它
    if (cur_weight + items[idx].weight <= capacity) {
        cur_selection[items[idx].id] = 1;
        dfs_backtrack(idx + 1, n, capacity, items, cur_selection, cur_value + items[idx].value,
                     cur_weight + items[idx].weight, best_value, best_selection, visited_states);
        cur_selection[items[idx].id] = 0;  // 回溯
    }
}

// 回溯法主函数：调用递归函数并返回结果
double test_backtracking(int n, int C, Item *items, int *selected, int *visited_states) {
    double best_value = 0;
    int *best_selection = calloc(n, sizeof(int));  // 存储最优选择
    int *cur_selection = calloc(n, sizeof(int));   // 当前递归路径的选择情况
    *visited_states = 0;

    // 开始深度优先搜索
    dfs_backtrack(0, n, C, items, cur_selection, 0.0, 0, &best_value, best_selection, visited_states);

    // 复制最终选择到输出数组
    memcpy(selected, best_selection, n * sizeof(int));

    free(best_selection);
    free(cur_selection);
    return best_value;
}

// 主函数：测试不同规模下各算法表现
int main(void) {
    srand(time(NULL));

    // 测试物品数量列表和容量值
    int n_list[] = {10, 20, 30, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 40000, 80000, 160000, 320000};
    int C_list[] = {10000, 100000, 1000000};
    int num_n = sizeof(n_list) / sizeof(n_list[0]);
    int num_C = sizeof(C_list) / sizeof(C_list[0]);

    // 打开结果文件
    FILE *fp = fopen("final_results.csv", "w");
    if (!fp) {
        perror("打开结果文件时出错");
        return 1;
    }
    fprintf(fp, "算法,n,C,价值,时间_毫秒,访问状态数\n");

    // 遍历每个 n 和 C 组合进行测试
    for (int c_idx = 0; c_idx < num_C; c_idx++) {
        int C = C_list[c_idx];
        for (int n_idx = 0; n_idx < num_n; n_idx++) {
            int n = n_list[n_idx];
            printf("处理中 n=%d, C=%d...\n", n, C);

            // 生成物品数据
            Item *items = (Item *)malloc(n * sizeof(Item));
            generate_items(items, n);

            // 保存1000个物品的数据
            if (n == 1000 && c_idx == 0) {
                item_data_save(items, n, "items.csv");
                printf("已将1000个物品保存到items.csv\n");
            }

            int *selected = (int *)malloc(n * sizeof(int));
            double value;
            clock_t start, end;
            int visited_states;
            double time_ms;

            // 贪心法（所有情况下均运行）
            memset(selected, 0, n * sizeof(int));
            start = clock();
            value = test_greedy(n, C, items, selected, &visited_states);
            end = clock();
            time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
            fprintf(fp, "贪心法,%d,%d,%.2f,%.2f,%d\n", n, C, value, time_ms, visited_states);
            printf("  贪心法: 价值=%.2f, 时间=%.2f 毫秒\n", value, time_ms);

            // 动态规划（有限制规模的运行）
            if (n <= 10000 && C == 10000) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                value = test_dp(n, C, items, selected, &visited_states);
                end = clock();
                time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
                fprintf(fp, "动态规划,%d,%d,%.2f,%.2f,%d\n", n, C, value, time_ms, visited_states);
                printf("  动态规划: 价值=%.2f, 时间=%.2f 毫秒\n", value, time_ms);
            }

            // 回溯法（规模小）
            if (n <= 30) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                value = test_backtracking(n, C, items, selected, &visited_states);
                end = clock();
                time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
                fprintf(fp, "回溯法,%d,%d,%.2f,%.2f,%d\n", n, C, value, time_ms, visited_states);
                printf("  回溯法: 价值=%.2f, 时间=%.2f 毫秒\n", value, time_ms);
            }

            // 蛮力法（规模小）
            if (n <= 30) {
                memset(selected, 0, n * sizeof(int));
                start = clock();
                value = test_brute_force(n, C, items, selected, &visited_states);
                end = clock();
                time_ms = ((double)(end - start)) * 1000 / CLOCKS_PER_SEC;
                fprintf(fp, "蛮力法,%d,%d,%.2f,%.2f,%d\n", n, C, value, time_ms, visited_states);
                printf("  蛮力法: 价值=%.2f, 时间=%.2f 毫秒\n", value, time_ms);
            }

            free(items);
            free(selected);
        }
    }

    fclose(fp);
    printf("所有测试已完成。结果已保存到final_results.csv\n");
    return 0;
}