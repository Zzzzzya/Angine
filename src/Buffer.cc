#include "Buffer.hpp"

FrameBuffer::FrameBuffer(int imageWidth, int imageHeight) {

    setUpFrameBuffer(imageWidth, imageHeight);
}

void FrameBuffer::setUpFrameBuffer(int imageWidth, int imageHeight) {
    // 帧缓冲
    // 1. 创建一个帧缓冲
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // 2. 创建一个纹理图像 - 这个帧缓冲会渲染到这个纹理图像上
    unsigned int texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 NULL); // 开内存 没赋值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    // 3.将纹理附加到当前帧缓冲上，这样帧缓冲就会渲染到这个纹理上了
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
    // 4.创建一个渲染缓冲
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, imageWidth, imageHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // 5.把渲染缓冲对象附加到帧缓冲的深度和模板附件上
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // 6.检查帧缓冲是否完整 否则打印错误信息。
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    FrameBufferID = framebuffer;
    TextureColorBuffer = texColorBuffer;
    RBO = rbo;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBufferDepthMap::FrameBufferDepthMap(int imageWidth, int imageHeight) {
    setUpFrameBuffer(imageWidth, imageHeight);
}

void FrameBufferDepthMap::setUpFrameBuffer(int imageWidth, int imageHeight) {
    // 1. 创建一个帧缓冲
    GLuint FBO;
    glGenFramebuffers(1, &FBO);

    // 2. 创建一个2D纹理 供帧缓冲的深度缓冲使用
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, imageWidth, imageHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    // 我们不需要颜色缓冲 但是帧缓冲对象必须有一个完整的帧缓冲 所以我们创建一个不可写的颜色缓冲附件
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    depthMapFBO = FBO;
    depthMapTexture = depthMap;
}
