# 切线空间的转换

### 0.why

全局的法线贴图往往存储的是一个世界坐标下固定的法线方向，但是这样做会存在一个问题：当Model的UV展开图存在重叠、旋转或者镜像时，同一个纹理像素会对应模型上的多个不同物理朝向区域。因此我们应该放弃使用全局空间坐标下的法线，转而为模型表面的每个点或渲染片元定义一个局部的、随表面移动的坐标系，即切线空间。这样做就可以允许UV展开图自由地重叠、旋转和镜像等操作。

### 1.what

切线空间是一个由三个相互正交的向量构成的局部坐标系，分别是法线（Normal，N），切线坐标（Tangent，T）与副切线坐标（Bitangent，B）。其中T、B方向分别对应了纹理坐标的U、V方向。使用时，应该将实际的法线方向从该片元的切线空间恢复至世界空间。

### 2.how

每一次转换，我们都是已知一个空间上的三角形的三个点的三维坐标与他们对应的UV二维坐标。分别记作P，U。那么他们的边向量可以表示如下

$$
\vec{e_0}=P_1-P_0\\
\vec{e_1}=P_2-P_0\\
\vec{u_0}=U_1-P_0\\
\vec{u_1}=U_2-P_0\\
$$

其中e为三维向量，u为二维向量。那么可知存在一个2*3的矩阵M，有以下成立

$$
M\times \begin{pmatrix}
\vec{e_0} & \vec{e_1}
\end{pmatrix}
=\begin{pmatrix}
\vec{u_0} & \vec{u_1}
\end{pmatrix}
$$

我们记3\*2矩阵E，2\*2矩阵U为上面分别两个矩阵

$$
E=\begin{pmatrix}
\vec{e_0} & \vec{e_1}
\end{pmatrix}
\\
U=\begin{pmatrix}
\vec{u_0} & \vec{u_1}
\end{pmatrix}
$$

那么（2）式可以这样表示

$$
M\times E=U
$$

我们想找的切线空间向量应该是可以把它们映射到UV坐标的，如下所示

$$
M\vec{t}=\begin{pmatrix}
1\\0
\end{pmatrix}
\\
M\vec{b}=\begin{pmatrix}
0\\1
\end{pmatrix}
$$

也可以写作

$$
M\times\begin{pmatrix}
\vec{t} & \vec{b}
\end{pmatrix}
=\begin{pmatrix}
1 & 0\\0 & 1
\end{pmatrix}
$$

而根据公式（4），有以下

$$
M\times E\times U^{-1}
=\begin{pmatrix}
1 & 0\\0 & 1
\end{pmatrix}
$$

由此可知

$$
\begin{pmatrix}
\vec{t} & \vec{b}
\end{pmatrix}
=E\times U^{-1}
$$

最后加上由三点法线插值得到的法线向量n，完整的切线空间就可以表示如下

$$
\begin{pmatrix}
\vec{t} & \vec{b}& \vec{n}
\end{pmatrix}
$$
