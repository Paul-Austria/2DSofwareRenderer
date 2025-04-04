#ifndef RENDERERBASE_H
#define RENDERERBASE_H


#define MAXBYTESPERPIXEL 16
#define MAXROWLENGTH 2048

namespace Tergos2D{
    class RenderContext2D;
    
    class RendererBase
    {
    public:
        RendererBase(RenderContext2D& context);
        ~RendererBase() = default;

        
    protected:
        RenderContext2D& context;
    };
    
}

#endif // !RENDERERBASE_H