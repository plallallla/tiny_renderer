# 常见变换

#### 0.Why

在图形真正渲染前，我们需要将模型数据、场景变换、用户输入等其他内容进行综合，因此需要将模型上的点转换为实际屏幕上输出的点。比如移动一个苹果，压扁它，从不同的角度观察等，都会使得原始的模型数据发生变化，这一系列变化都可以通过变化来实现。

#### 1.What

我们认为，变换就是将一个已知条件中的点通过一系列操作后变为渲染时实际需要的点。在这个过程中，输入可以是一个点、一个向量，输出应该与这个点、向量对应。
$$
p^{\prime} = Mp
$$

#### 2.How

##### 2.0 齐次坐标

我们使用列向量来表示一个点，空间上为确认一个点需要三个值，同时、为便于处理位移与Z缓存，我们使用齐次坐标来记录一个点，也就是用四个值来确认空间中的一个点
$$
p=
\begin{bmatrix}
x\\
y\\
z\\
w
\end{bmatrix}
$$

##### 2.1 平移

平移描述从一个位置到另一个位置的变化
$$
T=T(t_x,t_y,t_z)=
\begin{bmatrix}
1&0&0&t_x\\
0&1&0&t_y\\
0&0&1&t_z\\
0&0&0&1
\end{bmatrix}
$$
当我们描述一个位移，可以表述为以下的形式
$$
p^{\prime} = Tp
=\begin{bmatrix}
1 & 0 & 0 & t_x \\
0 & 1 & 0 & t_y \\
0 & 0 & 1 & t_z \\
0 & 0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
x \\
y \\
z \\
1
\end{bmatrix}
=
\begin{bmatrix}
x + t_x \\
y + t_y \\
z + t_z \\
1
\end{bmatrix}
$$

##### 2.2 旋转

首先，我们从二维平面上的点 (x,y)绕原点逆时针旋转θ角度的情况开始。原角度记作α，设旋转后的坐标为 (x′,y′)，则有
$$
\begin{cases}
x=rcos\alpha\\
y=rsin\alpha\\
\end{cases}
$$

$$
\begin{cases}
x^{\prime}=r\cos{(\alpha+\theta)}=r\cos\alpha\cos\theta-r\sin\alpha\sin\theta=x\cos\theta-y\sin\theta\\
y^{\prime}=r\sin{(\alpha+\theta)}=r\cos\alpha\sin\theta+r\sin\alpha\cos\theta=x\sin\theta+y\cos\theta
\end{cases}
$$

写作矩阵形式如下
$$
\begin{bmatrix}
x^{\prime}\\
y^{\prime}
\end{bmatrix}
=
\begin{bmatrix}
cos\theta&-sin\theta\\
sin\theta&cos\theta
\end{bmatrix}
\begin{bmatrix}
x\\
y
\end{bmatrix}
$$
推广到三维坐标后，我们可以有下面三个矩阵，分别对应绕X，Y，Z轴旋转
$$
\mathbf{R}_x(\phi) =
\begin{pmatrix}
1 & 0 & 0 & 0 \\
0 & \cos \theta & -\sin \theta & 0 \\
0 & \sin \theta & \cos \theta & 0 \\
0 & 0 & 0 & 1
\end{pmatrix}\\

\mathbf{R}_y(\phi) =
\begin{pmatrix}
\cos \theta & 0 & \sin \theta & 0 \\
0 & 1 & 0 & 0 \\
-\sin \theta & 0 & \cos \theta & 0 \\
0 & 0 & 0 & 1
\end{pmatrix}\\

\mathbf{R}_z(\phi) =
\begin{pmatrix}
\cos \theta & -\sin \theta & 0 & 0 \\
\sin \theta & \cos \theta & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{pmatrix}
$$

##### 2.3 缩放

缩放描述一个物体某个方向的轴变为原来的数倍
$$
S=S(s_x,s_y,s_z)=
\begin{bmatrix}
s_x&0&0&0\\
0&s_y&0&0\\
0&0&s_z&0\\
0&0&0&1
\end{bmatrix}
$$
当s_x= s_y= s_z的时候，这个缩放操作被称作均匀缩放，反之则称为非均匀缩放。

##### 2.4 切变

切变（剪切变换）用以描述对整个场景进行扭曲，他们可以代表一个值受到另一个坐标值的线性变化，比如
$$
H_{xz}(s)
=\begin{bmatrix}
1 & 0 & s & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$
我们实际应用一下这个矩阵，可以得到以下结果
$$
p^{\prime} = H_{xz}p
=\begin{bmatrix}
1 & 0 & s & 0 \\
0 & 1 & 0 & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
x \\
y \\
z \\
1
\end{bmatrix}
=
\begin{bmatrix}
x + s \dot z \\
y \\
z \\
1
\end{bmatrix}
$$
可以看到，这个点在x坐标发生了偏差，且这个偏差大小与他的z坐标有关，这种变化就是切变，我们有以下六种基本切变，其中第一个下标用于表示哪个坐标会被剪切矩阵改变，第二个下标代表将会使用哪个坐标来进行剪切
$$
\mathbf{H}_{xy}(s)\\\mathbf{H}_{xz}(s)\\\mathbf{H}_{yx}(s)\\\mathbf{H}_{yz}(s\\\mathbf{H}_{zx}(s)\\\mathbf{H}_{zy}(s)
$$
此外，我们还可以有以下看起来更复杂的切变矩阵它们代表了这两个坐标x,y都会被第三个坐标z剪切，他本质上是以上基本切变组合而来
$$
H_{xy}(s,t)
=\begin{bmatrix}
1 & 0 & s & 0 \\
0 & 1 & t & 0 \\
0 & 0 & 1 & 0 \\
0 & 0 & 0 & 1
\end{bmatrix}
$$
同时，由于切变应该是一种体积保持（volume-preserving）的变换，也就是二维上面积不变、三维上体积不变，对应任何任何剪切矩阵的行列式值都为1
$$
\vert \mathbf{H} \vert =1
$$


##### 附.1

观察2.1与2.2的变换矩阵，我们可以看到他们有一些相似之处，这是因为坐标平移本质上可以看作上更高一维上切变后在原维度的投影，也就是说，二维的平移与三维的切变可以在某个视角下是等价的，参考[pikuma](https://www.youtube.com/@pikuma)的视频[Math for Game Developers: Why do we use 4x4 Matrices in 3D Graphics?](https://www.youtube.com/watch?v=Do_vEjd6gF0&list=PLYnrabpSIM-97qGEeOWnxZBqvR_zwjWoo&index=1)
