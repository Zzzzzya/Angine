# 目录
- [目录](#目录)
- [Angine](#angine)
- [效果展示](#效果展示)
  - [Games202](#games202)
    - [homework0](#homework0)
    - [homework1](#homework1)
  - [learnOpengl](#learnopengl)
    - [Basic 基础功能](#basic-基础功能)
    - [高级Opengl篇](#高级opengl篇)
    - [高级光照](#高级光照)
    - [PBR](#pbr)

# Angine
An render engine using opengl and imgui. Just for learning!

# 效果展示
## Games202
### homework0
> 环境测试  我就是移植到了自己的渲染器上 \
> 简单测试了PhongShader & PCF
> 
![img](res/output/gifs/games202/homework0.gif)

### homework1
- 1. shadowMap
  ![img](res/output/gifs/games202/homework1/1.png)
  > ! 有明显的锯齿边
  ![img](res/output/gifs/games202/homework1/2.png)
- 2. PCF
  ![img](res/output/gifs/games202/homework1/poisson.png)
- 3. PCSS
  ![img](res/output/gifs/games202/homework1/pcss.png)

## learnOpengl
### Basic 基础功能
- Select Model (模板测试)
  ![img](res/output/gifs/border.jpg)
- BlinnPhone 着色
  ![img](res/output/gifs/Blinn-phone.gif)
- Z-depth
  ![img](res/output/gifs/Z-depth.jpg)
- Normal
  ![img](res/output/gifs/Normal.jpg)

### 高级Opengl篇
- 帧缓冲
  - 模糊
    ![img](res/output/gifs/blur.jpg)
  - 锐化
    ![img](res/output/gifs/Sharpen.jpg)
  - 灰度
    ![img](res/output/gifs/GreyScale.jpg)
  - 后视镜
    ![img](res/output/gifs/backmirror.gif)
- 面剔除
    ![img](res/output/gifs/image.png)
- 立方体贴图
  - 天空盒
    ![img](res/output/gifs/skybox.jpg)
  - 反射
    ![img](res/output/gifs/skybox.jpg)
  - 折射
    ![img](res/output/gifs/Refract.png)

### 高级光照
- 阴影
  - ShadowMapping
    ![img](res/output/gifs/ShadowMap.png)
  - PCF
    ![img](res/output/gifs/PCF.png)

### PBR
- 直接光照pbr
  ![img](res/output/gifs/pbr0.png)
  ![img](res/output/gifs/pbr_image.png)
- IBL
  - diffuse
    ![img](res/output/gifs/IBL_diffuse.png)
  - specular