#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>  // For std::cout
#include "../data/Texture.h"
#include "../data/Color.h"

class RenderContext2D
{

public:
    RenderContext2D() = default;
    ~RenderContext2D() = default;

    void SetTargetTexture(Texture* targettexture);
    void ClearTarget(Color color);


    void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height);
    void DrawArray(uint8_t* data,uint16_t x, uint16_t y, PixelFormat sourceFormat);


    static void Test() { std::cout << "hello world"; }

private:
    Texture* targetTexture = nullptr;
};


#endif