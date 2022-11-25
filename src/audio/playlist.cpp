#include "playlist.h"

#include <mathf.h>

#include <cassert>
#include <iostream>

void gbs_opus::playlist::init(std::vector<uint8_t> order)
{
    m_index = 0;
    m_list = std::move(order);
}

void gbs_opus::playlist::init(int track_count)
{
    assert(track_count > 0);

    std::vector<uint8_t> tempv;
    tempv.reserve(track_count);
    switch(m_mode)
    {
        case PlaylistMode::Forward:
            for (int i = 0; i < track_count; ++i)
                tempv.push_back(i);
            break;
        case PlaylistMode::Backward:
            for (int i = track_count - 1; i >= 0; --i)
                tempv.push_back(i);
            break;
        case PlaylistMode::Shuffle:
            for (int i = 0; i < track_count; ++i)
                tempv.push_back(i);
            shuffle_list();
            break;
        default:
            std::cerr << "Attempting to initialize a playlist with "
                                 "an unknown or unsupported PlaylistMode. "
                                 "Defaulting to Forward\n";
            m_mode = PlaylistMode::Forward;
            for (int i = 0; i < track_count; ++i)
                tempv.push_back(i);
            break;
    }

    m_index = 0;
    m_list.swap(tempv);
    m_count = track_count;
}

int gbs_opus::playlist::next_track()
{
    return m_list[++m_index];
}

int gbs_opus::playlist::prev_track() {
    return m_list[--m_index];
}

int gbs_opus::playlist::goto_track(int index)
{
    if (m_mode == PlaylistMode::Shuffle)
    {
        if (index >= m_count || index < 0)
            shuffle_list();
    }

    m_index = sanitize_index(index);
    return m_list[m_index];
}

int gbs_opus::playlist::sanitize_index(int index) const
{
    if (index >= m_count) return 0;
    if (index < 0) return m_count - 1;
    return index;
}

int gbs_opus::playlist::track() const
{
    return m_list[m_index];
}

#include <random>
#include <algorithm>
static std::random_device rd;
static std::mt19937 g(rd());

void gbs_opus::playlist::shuffle_list()
{
    std::shuffle(m_list.begin(), m_list.end(), g);
}

void gbs_opus::playlist::mode(gbs_opus::PlaylistMode m)
{
    if (m == m_mode) return;

    // Reinit with new mode, but try to match track
    m_mode = m;
    auto last_track = track();
    init(m_count);

    for (int i = 0; i < m_count; ++i)
    {
        if (m_list[i] == last_track)
        {
            m_index = i;
            break;
        }
    }
}

int
gbs_opus::gbs_playlist::index() const
{
    return playlist::index();
}

gbs_opus::PlaylistMode
gbs_opus::gbs_playlist::mode() const
{
    return playlist::mode();
}

void
gbs_opus::gbs_playlist::mode(gbs_opus::PlaylistMode m)
{
    playlist::mode(m);
}

int gbs_opus::gbs_playlist::track() const {
    return playlist::track();
}

bool gbs_opus::gbs_playlist::init(const gbs_opus::gbs_meta *meta)
{
    if (!meta)
    {
        std::cerr << "Error: failed to initialize gbs_playlist: "
                     "meta was null\n";
        return false;
    }

    playlist::init((int)meta->num_tracks());
    m_meta = meta;

    return true;
}

int gbs_opus::gbs_playlist::goto_track(int index)
{
    if (!m_meta)
    {
        std::cerr << "Error: gbs_playlist::goto_track failed. Needs "
                     "to be init first.\n";
        return -1;
    }

    auto pl_track = playlist::goto_track(index);

    return m_meta->tracks()[pl_track].track;
}

int gbs_opus::gbs_playlist::prev_track()
{
    auto pl_track = playlist::prev_track();

    return m_meta->tracks()[pl_track].track;
}

int gbs_opus::gbs_playlist::next_track()
{
    auto pl_track = playlist::next_track();

    return m_meta->tracks()[pl_track].track;
}

size_t gbs_opus::gbs_playlist::size() const
{
    return m_meta->tracks().size();
}
