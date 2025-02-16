#ifndef PRIMITIVESRENDERER_H
#define PRIMITIVESRENDERER_H

#include "../RendererBase.h"
#include "../../data/Color.h"

namespace Renderer2D
{

    class PrimitivesRenderer : RendererBase
    {
    public:
        PrimitivesRenderer(RenderContext2D &context);
        ~PrimitivesRenderer() = default;

        void DrawRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height);
        void DrawRotatedRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height, float angle, int16_t offsetX = 0, int16_t offsetY = 0);

    private:
    };

} // namespace Renderer2D

#endif //