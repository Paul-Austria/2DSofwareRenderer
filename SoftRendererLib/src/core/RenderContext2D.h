#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>
#include "../data/Texture.h"
#include "../data/Color.h"
#include "../data/BlendMode/BlendMode.h"
#include <functional>
#include <cstdint>

namespace Renderer2D
{
    enum class SamplingMethod {
        NEAREST,  
        LINEAR    
    };

    using BlendFunction = std::function<Color(const Color &src, const Color &dst)>;
    class RenderContext2D
    {

    public:
        RenderContext2D();
        ~RenderContext2D() = default;

        void SetTargetTexture(Texture *targettexture);
        void ClearTarget(Color color);
        void SetBlendMode(BlendMode mode);

        void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height);
   //     void DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height, float angle);
        void DrawTexture(Texture &texture, uint16_t x, uint16_t y);
        void DrawTexture(Texture &texture, uint16_t x, uint16_t y, float angle, int32_t offsetX = 0, int32_t offsetY = 0);

        void EnableClipping(bool clipping);
        void SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);
        // Typedef for the custom blend function

        void DrawTexture(Texture& texture, uint16_t x, uint16_t y, 
                      float scaleX, float scaleY, SamplingMethod method);

    private:

        Texture *targetTexture = nullptr;
        BlendMode mode = BlendMode::NOBLEND;
        SelectedBlendMode blendmode = SelectedBlendMode::SIMPLE;
        // clipping area
        uint16_t startX, startY, endX, endY;
        bool enableClipping = false;
    };
}
#endif