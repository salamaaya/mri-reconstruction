%cd /content/gdrive/MyDrive/CS677/mri-reconstruction

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

df = pd.read_csv('results/runtimes.csv')
CPU_SINGLE_RUNTIME = 7 * 3600  # seconds

VERSIONS     = ['CUDA', 'Optimized CUDA - Shared', 'Optimized CUDA - Textured']
PALETTE_3    = ['#3498db', '#2ecc71', '#e67e22']   # blue, green, orange
PALETTE_CPU  = ['#e74c3c'] + PALETTE_3             # red prepended for CPU bar

df_single = df[df['File'] == 'file1001598'].copy()

# ── Graph 1 ── CPU vs all three CUDA versions (single sample bar chart) ────────

bar_data = pd.concat([
    pd.DataFrame([{'Version': 'CPU', 'Runtime (seconds)': CPU_SINGLE_RUNTIME}]),
    df_single[['Version', 'Runtime (seconds)']]
], ignore_index=True)

fig, ax = plt.subplots(figsize=(9, 5))
sns.barplot(data=bar_data, x='Version', y='Runtime (seconds)',
            palette=PALETTE_CPU, ax=ax)

for p in ax.patches:
    val = p.get_height()
    label = f'{val/3600:.1f} hrs' if val > 600 else f'{val:.2f} s'
    ax.annotate(label, (p.get_x() + p.get_width() / 2, p.get_height()),
                ha='center', va='bottom', fontsize=10, fontweight='bold')

ax.set_title('MRI Reconstruction Runtime: CPU vs CUDA Variants\n(size = 238080)', fontsize=13)
ax.set_xlabel('Implementation')
ax.set_ylabel('Runtime (seconds)')
ax.grid(axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph1_cpu_vs_gpu_bar.png', dpi=150)
plt.show()

# ── Graph 2 ── Runtime vs k-space size (all 3 CUDA versions) ──────────────────

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=df, x='Size', y='Runtime (seconds)',
             hue='Version', hue_order=VERSIONS,
             marker='o', palette=PALETTE_3, ax=ax)

ax.set_title('Runtime vs. K-space Size: CUDA Variants', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Runtime (seconds)')
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph2_runtime_vs_size.png', dpi=150)
plt.show()

# ── Graph 3 ── Speedup of both optimized versions over baseline ────────────────

pivot = df.pivot_table(index=['File', 'Size'], columns='Version',
                       values='Runtime (seconds)').reset_index()

pivot['Speedup - Shared']   = pivot['CUDA'] / pivot['Optimized CUDA - Shared']
pivot['Speedup - Textured'] = pivot['CUDA'] / pivot['Optimized CUDA - Textured']

speedup_long = pd.melt(
    pivot, id_vars=['File', 'Size'],
    value_vars=['Speedup - Shared', 'Speedup - Textured'],
    var_name='Variant', value_name='Speedup'
)

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=speedup_long, x='Size', y='Speedup',
             hue='Variant', marker='o',
             palette={'Speedup - Shared': '#2ecc71', 'Speedup - Textured': '#e67e22'},
             ax=ax)
ax.axhline(y=1.0, linestyle='--', color='gray', label='No improvement')

ax.set_title('Speedup over Baseline CUDA: Shared vs Textured Memory', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Speedup (×)')
ax.legend()
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph3_speedup_vs_size.png', dpi=150)
plt.show()

# ── Graph 4 ── Shared vs Textured direct ratio ────────────────────────────────
# Values > 1 mean Shared is faster; < 1 mean Textured is faster

pivot['Shared_over_Textured'] = pivot['Optimized CUDA - Textured'] / pivot['Optimized CUDA - Shared']

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=pivot, x='Size', y='Shared_over_Textured',
             marker='o', color='#9b59b6', ax=ax)
ax.axhline(y=1.0, linestyle='--', color='gray')
ax.fill_between(pivot['Size'], pivot['Shared_over_Textured'], 1.0,
                where=pivot['Shared_over_Textured'] >= 1.0,
                alpha=0.15, color='#2ecc71', label='Shared faster')
ax.fill_between(pivot['Size'], pivot['Shared_over_Textured'], 1.0,
                where=pivot['Shared_over_Textured'] < 1.0,
                alpha=0.15, color='#e67e22', label='Textured faster')

ax.set_title('Relative Performance: Shared vs Textured Memory\n(>1 = Shared wins, <1 = Textured wins)', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Speedup Ratio (Textured runtime / Shared runtime)')
ax.legend()
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph4_shared_vs_textured_ratio.png', dpi=150)
plt.show()

# ── Graph 5 ── Throughput vs size (all 3 CUDA versions) ───────────────────────

df['Throughput (pixels/sec)'] = df['Size'] / df['Runtime (seconds)']

fig, ax = plt.subplots(figsize=(9, 5))
sns.lineplot(data=df, x='Size', y='Throughput (pixels/sec)',
             hue='Version', hue_order=VERSIONS,
             marker='o', palette=PALETTE_3, ax=ax)

ax.set_title('Throughput vs. K-space Size: CUDA Variants', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Throughput (pixels/sec)')
ax.grid(linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph5_throughput_vs_size.png', dpi=150)
plt.show()

# ── Graph 6 ── Throughput grouped bar at selected representative sizes ─────────

all_sizes   = sorted(df['Size'].unique())
n           = len(all_sizes)
# pick ~4 evenly-spaced sizes: smallest, ~33%, ~66%, largest
sel_indices = sorted({0, n//3, 2*n//3, n-1})
sel_sizes   = [all_sizes[i] for i in sel_indices]

df_sel = df[df['Size'].isin(sel_sizes)].copy()
df_sel['Size'] = df_sel['Size'].astype(str)          # categorical for grouping

fig, ax = plt.subplots(figsize=(10, 5))
sns.barplot(data=df_sel, x='Size', y='Throughput (pixels/sec)',
            hue='Version', hue_order=VERSIONS,
            palette=PALETTE_3, ax=ax)

ax.set_title('Throughput at Representative K-space Sizes: CUDA Variants', fontsize=13)
ax.set_xlabel('K-space Size (pixels)')
ax.set_ylabel('Throughput (pixels/sec)')
ax.legend(title='Version')
ax.grid(axis='y', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig('results/graph6_throughput_grouped_bar.png', dpi=150)
plt.show()

