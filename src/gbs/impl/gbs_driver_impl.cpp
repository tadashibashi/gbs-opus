#include <cstdlib>
#include "gbs_driver_impl.h"

namespace gbs_opus {

    void gbs_driver::impl::init_buffer(long buffer_bytes) {
        close_buffer();

        buffer.data = static_cast<int16_t *>(malloc(buffer_bytes));
        buffer.pos = 0;
        buffer.bytes = buffer_bytes;
    }

    void gbs_driver::impl::close_buffer() {
        if (buffer.data)
        {
            free(buffer.data);
            buffer.data = nullptr;
            buffer.bytes = 0;
            buffer.pos = 0;
        }
    }

    void gbs_driver::impl::close_gbs() {
        if (gbs)
        {
            gbs_close(gbs);
            gbs = nullptr;
        }
    }

    void gbs_driver::impl::update_pause_time() {
        pause_wait_time.tv_nsec = refresh_delay * 1000000;
    }

} // gbs_opus