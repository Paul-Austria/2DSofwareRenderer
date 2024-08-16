#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include "PixelFormat.h"

class Texture
{
public:
    Texture(uint32_t width, uint32_t height, PixelFormat format);
    Texture(uint32_t width, uint32_t height, uint8_t* data, PixelFormat format);
    ~Texture();

    /// @brief Get Pointer of Texture
    /// @return uint8_t*
    uint8_t* GetData();

    
    /// @brief Get Format of Texture
    /// @return PixelFormat
    PixelFormat GetFormat();

    
    uint32_t GetWidth();
    uint32_t GetHeight();
    
private:
    uint8_t* data;
    PixelFormat format;
    bool storedLocally = false;
    uint32_t width, height;
};


#endif