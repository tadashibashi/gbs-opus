#include "image.h"

#include <SDL_gpu.h>
#include <iostream>

namespace gbs_opus {
    bool image::load(const char *path)
    {
        GPU_Image *temp_img = GPU_LoadImage(path);
        if (!temp_img)
        {
            std::cerr << "Error: failed to load img \"" <<
                path << "\": " << GPU_PopErrorCode().details << '\n';
            return false;
        }

        // Clear current image to set new one
        close();
        m_img = temp_img;
        return true;
    }

    bool image::load(SDL_RWops *rw)
    {
        auto temp_img = GPU_LoadImage_RW(rw, true);
        if (!temp_img)
        {
            std::cerr << "Error: failed to load img from rwops stream: " <<
                      GPU_PopErrorCode().details << '\n';
            return false;
        }

        close();
        m_img = temp_img;
        return true;
    }

    bool image::create(size_t width, size_t height, const std::vector<uint8_t> *pixels)
    {
        // Create blank texture of specified size
        auto temp_img = GPU_CreateImage(width, height, GPU_FORMAT_RGBA);
        if (!temp_img)
        {
            std::cerr << "Error: failed to create: " <<
                      GPU_PopErrorCode().details << '\n';
            return false;
        }

        // Emplace pixels, if any
        if (pixels && !pixels->empty())
        {
            auto size = GPU_Rect{0, 0, (float)width, (float)height};
            GPU_UpdateImageBytes(temp_img, &size, (const unsigned char *)pixels->data(), 4 * (int)width);
        }

        close();
        m_img = temp_img;
        return true;
    }

    void image::close()
    {
        if (m_img)
        {
            GPU_FreeImage(m_img);
            m_img = nullptr;
        }
    }

    uintptr_t image::id() const
    {
        return GPU_GetTextureHandle(m_img);
    }

    size_t image::width() const {
        return m_img->base_w;
    }

    size_t image::height() const {
        return m_img->base_h;
    }


} // gbs_opus