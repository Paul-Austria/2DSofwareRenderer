#ifndef TRANSFORMEDTEXTURERENDERER_H
#define TRANSFORMEDTEXTURERENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"
#include "../../data/Texture.h"
#include <functional>

namespace Renderer2D
{
    class TransformedTextureRenderer : RendererBase
    {
    public:
        TransformedTextureRenderer(RenderContext2D &context);
        ~TransformedTextureRenderer() = default;


        void DrawTexture(Texture &texture, uint16_t x, uint16_t y, float angle, int32_t offsetX = 0, int32_t offsetY = 0);
        
        void DrawTexture(Texture &texture, uint16_t x, uint16_t y,
                         float scaleX, float scaleY);

        void DrawTexture(Texture &texture, uint16_t x, uint16_t y,
                         float scaleX, float scaleY, float angle,
                         int32_t offsetX, int32_t offsetY);


    private:
    };

} // namespace Renderer2D

#endif //