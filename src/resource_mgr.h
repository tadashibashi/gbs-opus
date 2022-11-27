// TODO: Add gif support

#ifndef GBS_OPUS_RESOURCE_MGR_H
#define GBS_OPUS_RESOURCE_MGR_H
#include "image.h"
#include "gbs/ogbs.h"
#include <map>
#include <string>

namespace gbs_opus
{
    class resource_mgr {
    public:
        resource_mgr() : m_images() {}
        ~resource_mgr() { clear(); }

        // ===== Images =================================================================
        /// Load image and receive it. Receives nullptr if there was a problem.
        /// The resource_mgr caches the image, so please don't delete it or alter it.
        /// Calling load_image again with the same path will retrieve the cached image.
        /// Use unload_image to unload this image, and clear to unload all resources.
        image *load_image(const std::string &path);

        /// Unloads an image if it exists in the internal store.
        /// Returns true if unloaded, false if the image doesn't exist in the store.
        bool unload_image(image *img);

        /// Unloads an image via its path, if it exists in the internal store.
        bool unload_image(const std::string &path);
        bool contains_image(const std::string &path);

        // ===== OGBS Files ==============================================================
        ogbs *load_gbs(const std::string &path);

        bool unload_gbs(ogbs *gbs);
        bool unload_gbs(const std::string &path);
        bool contains_gbs(const std::string &path);

        /// Releases all loaded resources
        void clear();

    private:
        std::map<std::string, image *> m_images;
        std::map<std::string, ogbs *> m_gbs;
    };
}



#endif //GBS_OPUS_RESOURCE_MGR_H
