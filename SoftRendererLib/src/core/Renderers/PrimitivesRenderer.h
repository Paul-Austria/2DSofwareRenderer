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

    private:
    };

} // namespace Renderer2D

#endif //