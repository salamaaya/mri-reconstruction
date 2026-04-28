import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv('results/runtimes.csv')
CPU_SINGLE_RUNTIME = 7 * 3600  # seconds

df_single = df[df['File'] == 'file1001598'].copy()

'''Graph 1 — CPU vs CUDA vs CUDA-opt (single sample bar chart)'''

bar_data = pd.concat([
    pd.DataFrame([{'Version': 'CPU', 'Runtime (seconds)': CPU_SINGLE_RUNTIME}]),
    df_single[['Version', 'Runtime (seconds)']]
], ignore_index=True)

fig, ax = plt.subplots(figsize=(8, 5))
sns.barplot(data=bar_data, x='Version', y='Runtime (seconds)',
            palette=['#e74c3c', '#3498db', '#2ecc71'], ax=ax)

# Annotate each bar with its value
for p in ax.patches:
    val = p.get_height()
    label = f'{val/3600:.1f} hrs' if val > 600 else f'{val:.1f} s'
    ax.annotate(label, (p.get_x() + p.get_width() / 2, p.get_height()),
                ha='center', va='bottom', fontsize=11, fontweight='bold')

ax.set_title('MRI Reconstruction Runtime: CPU vs CUDA vs Optimized CUDA\n(size = 238080)', fontsize=13)
ax.set_xlabel('Implementation')
ax.set_ylabel('Runtime (seconds)')
ax.grid(axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph1_cpu_vs_gpu_bar.png', dpi=150)
plt.show()

'''Graph 2 — CUDA vs CUDA-opt across all sizes (runtime scaling)'''

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=df, x='Size', y='Runtime (seconds)',
             hue='Version', marker='o', palette=['#3498db', '#2ecc71'], ax=ax)

ax.set_title('Runtime vs. K-space Size: CUDA vs Optimized CUDA', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Runtime (seconds)')
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph2_runtime_vs_size.png', dpi=150)
plt.show()

'''Graph 3 — Speedup of CUDA-opt over CUDA vs size'''

pivot = df.pivot_table(index=['File', 'Size'], columns='Version',
                       values='Runtime (seconds)').reset_index()
pivot['Speedup'] = pivot['CUDA'] / pivot['Optimized CUDA']

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=pivot, x='Size', y='Speedup', marker='o', color='#9b59b6', ax=ax)
ax.axhline(y=1.0, linestyle='--', color='gray', label='No improvement')

ax.set_title('Speedup of Optimized CUDA over Baseline CUDA vs. K-space Size', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Speedup (×)')
ax.legend()
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph3_speedup_vs_size.png', dpi=150)
plt.show()

'''Graph 4 — Throughput vs size'''

df['Throughput (pixels/sec)'] = df['Size'] / df['Runtime (seconds)']

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=df, x='Size', y='Throughput (pixels/sec)',
             hue='Version', marker='o', palette=['#3498db', '#2ecc71'], ax=ax)

ax.set_title('Throughput vs. K-space Size: CUDA vs Optimized CUDA', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Throughput (pixels/sec)')
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph4_throughput_vs_size.png', dpi=150)
plt.show()
