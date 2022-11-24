#ifndef GBS_OPUS_IMAGE_H
#define GBS_OPUS_IMAGE_H
#include <cstdint>
#include <string>

class GPU_Image;
class SDL_RWops;

namespace gbs_opus {

    class image {
    public:
        image() : m_img() { }
        ~image() { close(); }
        bool create(size_t width, size_t height, const std::vector<uint8_t> *pixels);

        bool load(const std::string &path) { return load(path.c_str()); }
        bool load(const char *path);
        bool load(SDL_RWops *rw);
        void close();

        /// Gets the texture id used by the graphics library
        [[nodiscard]] uintptr_t id() const;
        /// Gets the internal GPU_Image object
        [[nodiscard]] GPU_Image *img() { return m_img; }

        [[nodiscard]] bool is_loaded() { return static_cast<bool>(m_img); }

    private:
        GPU_Image *m_img;
    };

} // gbs_opus

#endif //GBS_OPUS_IMAGE_H
