#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>  // For std::cout
#include "../data/Texture.h"
#include "../data/Color.h"
#include "../data/BlendMode.h"

class RenderContext2D
{

public:
    RenderContext2D() = default;
    ~RenderContext2D() = default;

    void SetTargetTexture(Texture* targettexture);
    void ClearTarget(Color color);
    void SetBlendMode(BlendMode mode);

    void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height);
    void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height, float angle);
    void DrawArray(uint8_t* data,uint16_t x, uint16_t y,uint16_t width, uint16_t height,  PixelFormat sourceFormat);
    void DrawArray(uint8_t* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angle);
    void DrawArray(uint8_t* data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angleDegrees, float pivotX, float pivotY);
    
    static void Test() { std::cout << "hello world"; }

private:
    Texture* targetTexture = nullptr;
    BlendMode mode = BlendMode::NOBLEND;
};


#endif