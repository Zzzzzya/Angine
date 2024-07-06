#pragma once

#include "Header.hpp"
#include "Texture.hpp"

class FrameBuffer {
  public:
    FrameBuffer(int imageWidth, int imageHeight);
    int FrameBufferID;
    int TextureColorBuffer;
    int RBO;

  private:
    virtual void setUpFrameBuffer(int imageWidth, int imageHeight);
};

class FrameBufferDepthMap {
  public:
    FrameBufferDepthMap(int imageWidth, int imageHeight);
    unsigned int depthMapFBO;
    unsigned int depthMapTexture;

  private:
    void setUpFrameBuffer(int imageWidth, int imageHeight);
};

class Shader;
class FrameBufferEnvCube {
  public:
    FrameBufferEnvCube();
    void shade(shared_ptr<Shader> &shader, unsigned int texId);
    unsigned int FBO;
    unsigned int RBO;
    unsigned int env;

  private:
    void setUp();
};

class GBuffer {
  public:
    GBuffer();
    // void shade(shared_ptr<Shader> &shader, unsigned int texId);
    unsigned int gBuffer;
    unsigned int gPosition;
    unsigned int gNormal;
    unsigned int gAlbedoSpec;
    unsigned int gShadowMap;
    unsigned int gViewPos;

  private:
    void setUp();
};