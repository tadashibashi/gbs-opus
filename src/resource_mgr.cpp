#include "resource_mgr.h"
#include <iostream>

gbs_opus::image *
gbs_opus::resource_mgr::load_image(const std::string &path)
{
    // check if we've already loaded it
    auto it = m_images.find(path);
    if (it != m_images.end())
        return it->second;

    // load a new one
    auto img = new image;
    if (!img->load(path))
    {
        delete img;
        return nullptr;
    }

    m_images[path] = img;

    return img;
}

bool gbs_opus::resource_mgr::unload_image(gbs_opus::image *img) {
    for (auto &i : m_images)
    {
        if (i.second == img)
        {
            return unload_image(i.first);
        }
    }

    std::cerr << "Attempted to unload an image, but it does not exist in the resource_mgr\n";
    return false;
}

bool gbs_opus::resource_mgr::unload_image(const std::string &path)
{
    auto it = m_images.find(path);
    if (it != m_images.end())
    {
        if (!it->second)
        {
            // Just in case, if for some reason the pointer is null...
            std::cerr << "Error: resource_mgr::unload_image, tried unloading image at \"" <<
                path << "\", but the internal cache contained a nullptr.\n";
            return false;
        }

        it->second->close();
        m_images.erase(it);
        return true;
    }

    std::cerr << "Attempted to unload image with path " << path <<
        ", but it does not exist in the resource_mgr\n";
    return false;
}

bool
gbs_opus::resource_mgr::contains_image(const std::string &path)
{
    return m_images.find(path) != m_images.end();
}

void
gbs_opus::resource_mgr::clear()
{
    /// Close all images
    for (auto &img : m_images)
    {
        if (img.second)
            img.second->close();
    }
    m_images.clear();
}
