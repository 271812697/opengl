# 简介

**This is my work for learning graphics rendering**

Link:https://github.com/271812697/opengl

 Watch Video demo :https://www.bilibili.com/video/BV1Dw411e7aa/?vd_source=7975919d79e304e89435052729a5f4d9

### What **Features in my work**

1. Skeleton animation
2. PBR IBL Lighting 
3. Shadow map pcf pcss vsm csm
4. Path tracing 
5. 2D Fluid Simulation
6. Tile Based Forward Rendering
7. Post Process
8. Engine Editor
9. GLTF Viewer
10. Polygon Demo for D5 D5 school recruitment
11. DX 11 Renderer for Visualization of experimental results of the paper 

## How to build 

- Visual Studio 2019 or greater
- Windows 10/11 with OpenGl4.6 support
- Cmake

```
git clone https://github.com/271812697/opengl.git
cd opengl/
mkdir Build
cd Build/
cmake ..
```

# Screenshots

## **Render the Scene By path Tracing or Forward**

1.**Path Tracing**

![image-20230723223149759](README.assets/image-20230723223149759.png)

 2**.****Forward Rendering******

![image-20230723223315971](README.assets/image-20230723223315971.png)



## Fluid simulation

![image-20230706192959316](README.assets/image-20230706192959316.png)



## Particle With Trail

![image-20230807214506809](README.assets/image-20230807214506809.png)

## tiled forward renderer

![image-20230710142321377](README.assets/image-20230710142321377.png)

## environment lighting (IBL)

![image-20230710142554961](README.assets/image-20230710142554961.png)

## disney principled BSDF

![image-20230710142735202](README.assets/image-20230710142735202.png)

![image-20230710142832334](README.assets/image-20230710142832334.png)

![image-20230710142947662](README.assets/image-20230710142947662.png)

## Skeleton Animation

![image-20230710143120456](README.assets/image-20230710143120456.png)

## PCSS Shadow and Animation

![image-20230710143252170](README.assets/image-20230710143252170.png)

## SHADOW MAP PCF PCSS VSM

![image-20230712001741920](README.assets/image-20230712001741920.png)

![image-20230712194416099](README.assets/image-20230712194416099.png)





## gltf Viewer

###  from Sketch fab

![image-20231205220727247](README.assets/image-20231205220727247.png)

![img](https://z13qslsjsa3.feishu.cn/space/api/box/stream/download/asynccode/?code=MWQ5MmU2MTNiOTk2Njc0MTRjNzc1MGJlMTQ3MmY5NGRfSFlsT214Qm1sdUMwVUJ5Wk9VbjdGTzBOSWNUN2ZLbmpfVG9rZW46U2pXaWJqSGhpb1ZUZEN4ZXZmU2NaYVdibkpkXzE3MDE3ODUxNTA6MTcwMTc4ODc1MF9WNA)![img](https://z13qslsjsa3.feishu.cn/space/api/box/stream/download/asynccode/?code=NzZhN2VlYTA1Njk2MGIxNDJhOTc5NWUyZGYxODlkMGJfcnF1aGRORU9xMk0zRDZPbUVVTWtXYXdsZmRlVjNWVGpfVG9rZW46SktLZmJpSldlb240MXh4NDQ0WWNYcTBDbjljXzE3MDE3ODUxNTA6MTcwMTc4ODc1MF9WNA)![img](https://z13qslsjsa3.feishu.cn/space/api/box/stream/download/asynccode/?code=ZTk4YWU1NGZjNTY2NDI4YzZjZTIxNmQxMDA3N2Y1NDJfWkZuN1hJSE9lV0V0SkZGSUZKbjNYNWV2WlR6Z2RRbmVfVG9rZW46UG9wc2I1Qlgzb2syTzJ4NkdQZWNtY3FybnpoXzE3MDE3ODUxNTA6MTcwMTc4ODc1MF9WNA)

### from my work by Path Tracing

![image-20231205221011279](README.assets/image-20231205221011279.png)

### from my work by forward render

![image-20231205221054024](README.assets/image-20231205221054024.png)

## DX11

### **Purpose**

This renderer was written to visualize the results of the experiment

**my paper is to Dectecing  points of interest on 3D model** 

**example show:**

![image-20231205221939774](README.assets/image-20231205221939774.png)

### 功能界面:

![image-20231205224706455](README.assets/image-20231205224706455.png)

### 可视化功能

提供了对三维模型兴趣点检测的可视化,用来可视化论文实验的结果并与Ground Truth对比：

1. 可以查看三维模型的概率分布
2. 兴趣点球面坐标分布
3. 点云绘制
4. pbr+IBL渲染、phong渲染、深度模式、线框模式
5. 自定义的线框绘制
6. 提供论文实验结果中间结果分析
7. 截屏保存可视化结果
8. 可视化标注模型的兴趣点制作数据集

#### 概率分布

![image-20231205224046241](README.assets/image-20231205224046241.png)

#### 兴趣点球面坐标分布

![image-20231205224131205](README.assets/image-20231205224131205.png)

#### 查看三维模型的兴趣点预测结果:

左边为方法预测，右边为ground truth

![image-20231205224227192](README.assets/image-20231205224227192.png)

![image-20231205224612704](README.assets/image-20231205224612704.png)

点云绘制

![image-20231205230831903](README.assets/image-20231205230831903.png)

![image-20231205230937326](README.assets/image-20231205230937326.png)

pbr+IBL渲染、phong渲染、深度模式、线框模式

![image-20231205231058914](README.assets/image-20231205231058914.png)

![image-20231205231147179](README.assets/image-20231205231147179.png)

![image-20231205231231526](README.assets/image-20231205231231526.png)

![image-20231205231314942](README.assets/image-20231205231314942.png)

截屏

![image-20231205231520607](README.assets/image-20231205231520607.png)

标注模型

![image-20231205231749940](README.assets/image-20231205231749940.png)

