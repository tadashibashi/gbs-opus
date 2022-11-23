#include "mathf.h"

namespace gbs_opus
{
    /// Helper to wrap a range of numbers (inclusive on both lower and upper bounds)
    int wrap(int kX, int const kLowerBound, int const kUpperBound)
    {
        int range_size = kUpperBound - kLowerBound + 1;

        if (kX < kLowerBound)
            kX += range_size * ((kLowerBound - kX) / range_size + 1);

        return kLowerBound + (kX - kLowerBound) % range_size;
    }

    int clamp(int x, int lowerbound, int upperbound) {
        return x < lowerbound ? lowerbound : (x > upperbound ? upperbound : x);
    }
}