#ifndef CONTEXT2D_H
#define CONTEXT2D_H

#include <iostream>
#include "../data/Texture.h"
#include "../data/Color.h"
#include "../data/BlendMode/BlendMode.h"
#include <functional>
#include <cstdint>
#include "Renderers/PrimitivesRenderer.h"
#include "Renderers/BasicTextureRenderer.h"
#include "Renderers/TransformedTextureRenderer.h"

#define MAXBYTESPERPIXEL 16
#define MAXROWLENGTH 2048


namespace Renderer2D
{
    enum class SamplingMethod
    {
        NEAREST,
        LINEAR
    };

    struct ClippingArea{
        uint16_t startX, startY, endX, endY;
    };

    class RenderContext2D
    {

    public:
        RenderContext2D();
        ~RenderContext2D() = default;

        PrimitivesRenderer primitivesRenderer;
        BasicTextureRenderer basicTextureRenderer;
        TransformedTextureRenderer transformedTextureRenderer;

        void SetTargetTexture(Texture *targettexture);
        Texture* GetTargetTexture();


        void SetBlendMode(BlendMode mode);
        BlendMode GetBlendMode();


        void SetSamplingMethod(SamplingMethod method);
        SamplingMethod GetSamplingMethod();
        

        void ClearTarget(Color color);
        void EnableClipping(bool clipping);
        bool IsClippingEnabled();
        void SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);
    	ClippingArea GetClippingArea();




    private:
        Texture *targetTexture = nullptr;
        BlendMode mode = BlendMode::NOBLEND;
        SelectedBlendMode blendmode = SelectedBlendMode::SIMPLE;
        SamplingMethod samplingMethod = SamplingMethod::NEAREST;

        // clipping area
        uint16_t startX, startY, endX, endY;
        ClippingArea clippingArea;
        bool enableClipping = false;
    };
}
#endif