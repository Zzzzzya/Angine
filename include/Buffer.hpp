#pragma once

#include "Header.hpp"

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