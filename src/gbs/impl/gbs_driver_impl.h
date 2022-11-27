#ifndef GBS_OPUS_GBS_DRIVER_IMPL_H
#define GBS_OPUS_GBS_DRIVER_IMPL_H
#include <gbs/gbs_driver.h>
#include <gbs/libgbs.hpp>
#include <audio/plugout_driver.h>

const long DefaultRefreshDelay = 17;

namespace gbs_opus {

    class gbs_driver::impl {
    public:
        impl() : gbs{}, buffer{nullptr, 0, 0},
            refresh_delay{DefaultRefreshDelay}, pause_wait_time{}, paused{true} {
            update_pause_time();
        }

        ~impl() { close(); } // just in case

    public:
        struct gbs *gbs;
        gbs_output_buffer buffer;
        plugout_driver plugout;
        long refresh_delay;
        timespec pause_wait_time;
        bool paused;

    public:
        /// Helper to set up the buffer, closes any previously opened buffers
        void init_buffer(long buffer_bytes);

        /// Cleanup all internals. Safe to call if impl is already closed.
        void close()
        {
            close_buffer();
            close_gbs();
            paused = true;

            refresh_delay = DefaultRefreshDelay;
            update_pause_time();
        }

        /// Cleanup buffer for reuse. Safe to call if buffer is already closed.
        void close_buffer();

        /// Cleanup gbs for reuse. Safe to call if gbs is already closed.
        void close_gbs();

    private:
        void update_pause_time();
    };

} // gbs_opus

#endif //GBS_OPUS_GBS_DRIVER_IMPL_H
