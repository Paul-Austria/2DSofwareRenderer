#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <sys/mman.h>
#include <chrono>
#include <thread>
#include <SoftRendererLib/src/include/SoftRenderer.h>
#include <SoftRendererLib/src/data/PixelFormat/PixelFormatInfo.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION

#include "lib/stb_image.h"
#include <fstream>

using namespace Tergos2D;

#define CHECK_ERR(cond, msg) \
    if (cond)                \
    {                        \
        perror(msg);         \
        exit(EXIT_FAILURE);  \
    }

// Helper function to handle DRM events
void handle_drm_events(int drm_fd)
{
    drmEventContext evctx = {
        .version = DRM_EVENT_CONTEXT_VERSION,
        .vblank_handler = nullptr,
        .page_flip_handler = nullptr,
    };

    struct pollfd fds = {
        .fd = drm_fd,
        .events = POLLIN,
    };

    while (poll(&fds, 1, -1) <= 0)
        ; // Wait for events

    if (fds.revents & POLLIN)
    {
        drmHandleEvent(drm_fd, &evctx);
    }
}

// Helper function to create a framebuffer
uint8_t *create_framebuffer(int drm_fd, uint32_t width, uint32_t height, uint32_t bpp, uint32_t &fb_id, uint32_t &handle, uint32_t &pitch, uint64_t &size)
{
    struct drm_mode_create_dumb create_dumb = {0};
    create_dumb.width = width;
    create_dumb.height = height;
    create_dumb.bpp = bpp;
    CHECK_ERR(drmIoctl(drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) < 0, "Failed to create dumb buffer");

    handle = create_dumb.handle;
    pitch = create_dumb.pitch;
    size = create_dumb.size;

    // Map the dumb buffer to memory
    struct drm_mode_map_dumb map_dumb = {0};
    map_dumb.handle = handle;
    CHECK_ERR(drmIoctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) < 0, "Failed to map dumb buffer");

    uint8_t *pixels = (uint8_t *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map_dumb.offset);
    CHECK_ERR(pixels == MAP_FAILED, "Failed to mmap dumb buffer");

    // Create framebuffer object
    CHECK_ERR(drmModeAddFB(drm_fd, width, height, 24, 24, pitch, handle, &fb_id) < 0, "Failed to create framebuffer");

    return pixels;
}

double getCurrentTime()
{
    // Get the current time since epoch in seconds as a floating-point value
    using Clock = std::chrono::steady_clock; // Use steady_clock for consistent timing
    static auto startTime = Clock::now();    // Store the starting time point
    auto currentTime = Clock::now();         // Get the current time point
    auto duration = std::chrono::duration<double>(currentTime - startTime);
    return duration.count(); // Return time in seconds
}

int main()
{
    int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    CHECK_ERR(drm_fd < 0, "Failed to open /dev/dri/card0");

    // Get DRM resources
    drmModeRes *resources = drmModeGetResources(drm_fd);
    CHECK_ERR(!resources, "Failed to get DRM resources");

    // Select the first connected connector
    drmModeConnector *connector = nullptr;
    uint32_t connector_id = 0;
    for (int i = 0; i < resources->count_connectors; i++)
    {
        connector = drmModeGetConnector(drm_fd, resources->connectors[i]);
        if (connector && connector->connection == DRM_MODE_CONNECTED)
        {
            connector_id = connector->connector_id;
            break;
        }
        drmModeFreeConnector(connector);
    }
    CHECK_ERR(connector_id == 0, "No connected connector found");

    // Choose the first valid mode (resolution)
    drmModeModeInfo mode = connector->modes[0];
    uint32_t width = mode.hdisplay;
    uint32_t height = mode.vdisplay;

    // Get encoder
    drmModeEncoder *encoder = drmModeGetEncoder(drm_fd, connector->encoder_id);
    CHECK_ERR(!encoder, "Failed to get encoder");

    // Get CRTC
    drmModeCrtc *crtc = drmModeGetCrtc(drm_fd, encoder->crtc_id);

    // Create double buffers
    uint32_t fb_id[2], handle[2], pitch[2];
    uint64_t size[2];
    uint8_t *framebuffer[2];

    for (int i = 0; i < 2; ++i)
    {
        framebuffer[i] = create_framebuffer(drm_fd, width, height, 24, fb_id[i], handle[i], pitch[i], size[i]);
    }

    // Set up the rendering context
    RenderContext2D context;

    bool running = true;
    int current = 0;

    // Set initial CRTC
    CHECK_ERR(drmModeSetCrtc(drm_fd, crtc->crtc_id, fb_id[current], 0, 0, &connector_id, 1, &mode) < 0, "Failed to set CRTC");

    Texture text;
    Texture text2;
    Texture text3;
    int imgwidth, imgheight, nrChannels;
    uint8_t *data = nullptr;
    uint8_t *data2 = nullptr;
    uint8_t *data3 = nullptr;
    Texture text4;
    uint8_t *data4 = nullptr;
    Texture text5;
    uint8_t *data5 = nullptr;
    int imgwidth4 = 234;
    int imgheight4 = 243;

    data = stbi_load("data/img1.jpg", &imgwidth, &imgheight, &nrChannels, 3);
    text = Texture(imgwidth, imgheight, data, PixelFormat::RGB24, 0);
    data2 = stbi_load("data/Candera.png", &imgwidth, &imgheight, &nrChannels, 4);
    text2 = Texture(imgwidth, imgheight, data2, PixelFormat::RGBA8888, 0);
    data3 = stbi_load("data/logo-de.png", &imgwidth, &imgheight, &nrChannels, 4);
    text3 = Texture(imgwidth, imgheight, data3, PixelFormat::RGBA8888, 0);
    data5 = stbi_load("data/images_small.png", &imgwidth, &imgheight, &nrChannels, 3);
    text5 = Texture(imgwidth, imgheight, data5, PixelFormat::RGB24, 0);
    // Load the binary file
    std::ifstream file("data/testrgb565.bin", std::ios::binary | std::ios::ate);
    if (file)
    {
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        data4 = new uint8_t[size];
        if (file.read(reinterpret_cast<char *>(data4), size))
        {
            // Successfully read binary data
            text4 = Texture(imgwidth4, imgheight4, data4, PixelFormat::RGB565, 0);
        }
        else
        {
            std::cerr << "Failed to read testrgb565.bin" << std::endl;
        }
        file.close();
    }
    else
    {
        std::cerr << "Failed to open testrgb565.bin" << std::endl;
    }

    context.SetBlendMode(BlendMode::SIMPLE);
    double previousTime = 0.0;
    int frameCount = 0;
    float x = 0.0f;
    while (running)
    {
        // Render to the back buffer
        int next = 1 - current;

        // Use the framebuffer directly as the texture
        Texture texture = Texture(width, height, framebuffer[next], PixelFormat::BGR24, pitch[next]);
        context.SetTargetTexture(&texture);
        // Clear and draw using RenderContext2D
        context.SetClipping(80, 30, 170, 290);
        context.EnableClipping(false);
        context.ClearTarget(Color(150, 150, 150));
        Coloring st = {false, Color(155, 0, 255, 0)};
        context.SetColoringSettings(st);

        for (size_t i = 0; i < 1; i++)
        {

            for (size_t y = 0; y < 5; y++)
            {

                for (int16_t x = 0; x < 10; x++)
                {
                    context.scaleTextureRenderer.DrawTexture(text5, x * 51, y * 52, 1, 1);
                }
            }
            /* code */
        }

        context.GetColoring().colorEnabled = false;
        //  context.basicTextureRenderer.DrawTexture(text4, 400, 130);
        context.SetBlendFunc(BlendFunctions::BlendSolidRowRGB24);
        context.primitivesRenderer.DrawRect(Color(130, 90, 90, 90), 0, 0, width, height);
        context.SetBlendFunc(BlendFunctions::BlendRGBA32ToRGB24);
        context.transformedTextureRenderer.DrawTexture(text2, 10, 10, 0);
        context.SetBlendFunc(BlendFunctions::BlendRow);
        // Perform a page flip to show the back buffer
        CHECK_ERR(drmModePageFlip(drm_fd, crtc->crtc_id, fb_id[next], DRM_MODE_PAGE_FLIP_EVENT, nullptr) < 0, "Failed to page flip");

        // Wait for the page flip event to complete
        handle_drm_events(drm_fd);

        x += 0.5f;
        // Switch buffers
        current = next;

        double currentTime = getCurrentTime();
        frameCount++;

        // Check if a second has passed
        if (currentTime - previousTime >= 1.0)
        {
            double fps = frameCount / (currentTime - previousTime);
            double timePerFrame = 1000.0 / fps; // Convert to milliseconds
            std::cout << "FPS: " << fps << " | Time per frame: " << timePerFrame << " ms" << std::endl;

            // Reset counters
            previousTime = currentTime;
            frameCount = 0;
        }
    }

    // Cleanup
    for (int i = 0; i < 2; ++i)
    {
        munmap(framebuffer[i], size[i]);
        drmModeRmFB(drm_fd, fb_id[i]);
        struct drm_mode_destroy_dumb destroy_dumb = {0};
        destroy_dumb.handle = handle[i];
        drmIoctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy_dumb);
    }

    drmModeFreeCrtc(crtc);
    drmModeFreeEncoder(encoder);
    drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    close(drm_fd);

    return 0;
}
