#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream> // For std::cout
#include "../data/Texture.h"
#include "../data/Color.h"
#include "../data/BlendMode.h"

namespace Renderer2D
{

    class RenderContext2D
    {

    public:
        RenderContext2D() = default;
        ~RenderContext2D() = default;

        void SetTargetTexture(Texture *targettexture);
        void ClearTarget(Color color);
        void SetBlendMode(BlendMode mode);

        void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height);
        void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height, float angle);
        void DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat);
        void DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angle);
        void DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angleDegrees, float pivotX, float pivotY);

        static void Test() { std::cout << "hello world"; }

        void EnableClipping(bool clipping);
        void SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);

    private:
        Texture *targetTexture = nullptr;
        BlendMode mode = BlendMode::NOBLEND;

        // clipping area
        uint16_t startX, startY, endX, endY;
        bool enableClipping;
    };
}
#endif