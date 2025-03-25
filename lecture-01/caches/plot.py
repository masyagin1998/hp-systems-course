import collections
import sys

import plotly.graph_objects as go

data_by_stride = collections.defaultdict(list)
for line in sys.stdin:
    size, stride, t = line.split(",")
    size, stride, t = int(size), int(stride), float(t)
    data_by_stride[stride].append((size, t))

fig = go.Figure()
for stride, vals in data_by_stride.items():
    x, y = zip(*vals)
    fig.add_trace(go.Scatter(x=x, y=y, mode="lines+markers", name=stride))

fig.update_layout(
    title="Measuring The Effects of CPU Caches",
    xaxis_title="Array Size (bytes)",
    yaxis_title="Average Memory Access Time (ns)",
    legend_title="Stride (bytes)",
)
fig.write_html("plot.html")