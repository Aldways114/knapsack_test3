import pandas as pd
import matplotlib.pyplot as plt
import matplotlib

# 设置中文字体和解决负号显示问题
matplotlib.rcParams['font.sans-serif'] = ['SimHei']
matplotlib.rcParams['axes.unicode_minus'] = False

# 读取CSV文件
file_path = "final_results.csv"
df = pd.read_csv(file_path)

# 过滤掉不完整的行
df = df.dropna(subset=['时间_毫秒'])

# 按算法分类提取数据
algorithms = ['贪心法', '动态规划', '回溯法', '蛮力法']
data_by_alg = {alg: df[df['算法'] == alg] for alg in algorithms}

# 创建4个子图
fig, axes = plt.subplots(2, 2, figsize=(14, 10), sharex=False, sharey=False)

# 设置子图标题与数据对应关系
for ax, (alg, data) in zip(axes.flatten(), data_by_alg.items()):
    n_values = data['n'].values
    time_values = data['时间_毫秒'].values

    ax.plot(n_values, time_values, marker='o', linestyle='-', linewidth=2, color='b')

    ax.set_title(f"{alg}", fontsize=14)
    ax.set_xlabel("物品数量 n", fontsize=12)
    ax.set_ylabel("运行时间（毫秒）", fontsize=12)
    ax.grid(True, which="both", ls="--")
    ax.set_xscale('log')
    ax.set_yscale('log')

# 自动调整布局
plt.tight_layout()
# 保存图像到本地
plt.savefig("algorithm_comparison_subplots.png", dpi=300, bbox_inches='tight') # PNG格式
plt.show()