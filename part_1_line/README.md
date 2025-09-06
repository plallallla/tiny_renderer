# Bresenham算法的两种推导方式

### 0.问题描述

对于直线绘制时，确定最大步进方向后，另一个方向何时步进，我们可以用Bresenham算法进行解决。在以下推导中，我们不妨假定最大步进方向始终为X轴方向，起点位于左下，终点位于右上。

### 1.中点推导

对于一般如下的直线公式
$$
f(x,y) = y - kx - b\\
k = \frac{\Delta y}{\Delta x}
$$
我们记起点为
$$
(x_0,y_0)
$$
对应有
$$
f(x_0,y_0) = y_0 - kx_0 - b =0
$$
当起点绘制结束后，X轴方向进行步进，而对于Y轴方向，我们不确定是否需要步进，不妨考虑下两个候选点的中点
$$
x_1 = x_0 + 1\\
y_1 = y_0 + 0.5
$$
此时我们考虑该中点的值，记为d0。
$$
d_0=f(x_1,y_1)\\
= f(x_0 + 1, y_0 + 0.5)\\
=y_0+0.5-k(x_0+1)-b\\
=0.5-k+f(x_0,y_0)\\
=0.5-k
$$
若中点值d大于0，则说明直线处于中点下方，更靠近下方的点，选取的下一个点应为右侧的点，Y轴方向不步进，对应的下一个需要考虑的中点为X轴步进1步，Y轴步进0.5步，对应中点值为d1如下
$$
d_0>0:\\
(x_1,y_1)=(x_0+1,y_0)\\
d_1=f(x_2,y_2)\\
=f(x_1+1,y_1+0.5)\\
=f(x_0+2,y_0+0.5)\\
=y_0+0.5-k(x_0+2)-b\\
=0.5-2k+f(x_0,y_0)\\
=d_0-k
$$
反之，若中点值小于0(等于0时两者皆可，我们考虑上方的点)，则说明直线处于中点上方，更靠近上方的点，Y轴方向也需要步进，此时下一个中点值d1应如下
$$
d_0\le 0:\\
(x_1,y_1)=(x_0+1,y_0+1)\\
d_1=f(x_2,y_2)\\
=f(x_1+1,y_1+0.5)\\
=f(x_0+2,y_0+1.5)\\
=y_0+1.5-k(x_0+2)-b\\
=1.5-2k+f(x_0,y_0)\\
=d_0+1-k
$$
综合(8)，(9)，(10)，关于中点值d我们有如下推论
$$
d_0=0.5-k\\
\text d_{n+1} = 
\begin{cases}
d_n-k, & d_n > 0 \\
d_n+1-k, & d_n \leq 0
\end{cases}\\
y_{n+1} = 
\begin{cases}
y_n, & d_n > 0 \\
y_n+1, & d_n \leq 0
\end{cases}
$$
对应实际代码中，我们再考虑上面的递推关系，不难发现其中的0.5与k都是需要进行除法运算，而我们始终只要进行考虑中点值的正负，因此可以考虑根据直线函数，公式（1）将中点值放大，记作D
$$
D=d * 2 * \Delta k\\
$$
这样更新后，有以下递推关系
$$
D_0=1-2\Delta y\\
\text D_{n+1} = 
\begin{cases}
D_n-2\Delta y, & d_n > 0 \\
D_n+2\Delta x-2\Delta y, & d_n \leq 0
\end{cases}\\
y_{n+1} = 
\begin{cases}
y_n, & D_n > 0 \\
y_n+1, & D_n \leq 0
\end{cases}
$$
对应代码如下

```c++
    int y = ay;
    int d = 1 - 2 * abs(by - ay);//D0 = 1 - 2*dy
    for (int x = ax; x <= bx; x++)
    {
        if (steep) // 绘制
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        d -= 2 * abs(by - ay);//D0 > 0时,D1 = D0 - 2dy, y方向不变
        if (d < 0)//D0 < 0时,D1 = D0 + 2dx - 2dy,y方向上进一
        {
            y += (by > ay ? 1 : -1);
            d += 2 * bx - ax;
        }
    }
```

### 2.偏差值推导

如果考虑Y轴上实际的步进方向，那么根据直线函数，公式（1），有如下递推关系
$$
x_{n+1}=x_n+1\\
y_{n+1}=y_n+k
$$
可以看到，Y轴上每次的步进大小均为k，如果用d表示实际交点在Y轴上的偏差，则有
$$
d_0=0\\
d_{n+1}=d_n+k
$$
考虑从起点出发后的第一个点
$$
(x_0,y_0) \to (x_1,y_1)=(x_0+1,y_0+d_1)=(x_0+1,y_0+k)
$$
当d>0.5时，说明交点偏上，应取上一个点，反之取下面的点
$$
y_1 = 
\begin{cases}
y_0+1, &d_1>0.5\\
y_0, &d_1\le0.5
\end{cases}
$$
但当我们考虑再下一个点时，会发现，若d>0.5时，则在后续递增，d会始终大于0.5，每一步都会在Y轴上递进，此时我们需要将d进行修正，最终d的递推关系如下
$$
d_0=0\\
d_{n+1}=
\begin{cases}
d_n+k-1, &d_n>0.5\\
d_n+k, &d_n\le0.5
\end{cases}
$$
对应实际代码中，我们同样可以对d进行调整，从而优化到整数计算，同样将偏差值放大2*dx，记作D，此时
$$
D_0=0\\
D_{n+1}=
\begin{cases}
D_n+2\Delta y-2\Delta x, &D_n>\Delta x\\
D_n+2\Delta y, &D_n\le\Delta x
\end{cases}\\
y_{n+1} = 
\begin{cases}
y_n+1, & D_n > \Delta x \\
y_n, & D_n \leq \Delta x
\end{cases}
$$
对应代码如下

```c++
    int y = ay;
    int d = 0;//考虑Y轴上的偏差值D
    for (int x = ax; x <= bx; x++)
    {
        if (steep)//绘制
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        d += 2 * abs(by - ay);
        if (d > bx - by)//D > dx，此时需要步进
        {
            y += (by > ay ? 1 : -1);
            d -= 2 * (bx - ax);//步进后对偏差值作修正
        }
    }
```

实际开发过程中，我们还可以将dx移项，记作E，使得偏差值最终与0比较大小，参考如下
$$
E_0=-\Delta x\\
E_{n+1}=
\begin{cases}
E_n+2\Delta y-2\Delta x, &E_n>0\\
E_n+2\Delta y, &E_n\le0\end{cases}\\
y_{n+1} = \begin{cases}
y_n+1, & E_n>0\\
y_n, & E_n\le0\end{cases}
$$
对应代码如下
```C++
    int y = ay;
    int e = -1 * (bx - ax);
    for (int x = ax; x <= bx; x++)
    {
        if (steep)//绘制
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        e += 2 * abs(by - ay);
        if (e > 0)
        {
            y += (by > ay ? 1 : -1);
            e -= 2 * (bx - ax);
        }
    }
```
