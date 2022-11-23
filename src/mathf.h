//
// Created by Aaron Ishibashi on 11/21/22.
//

#ifndef GBS_OPUS_MATHF_H
#define GBS_OPUS_MATHF_H

namespace gbs_opus
{
    /// Helper to wrap a range of numbers (inclusive on both lower and upper bounds)
    int wrap(int kX, int kLowerBound, int kUpperBound);
    int clamp(int x, int lowerbound, int upperbound);
}

#endif //GBS_OPUS_MATHF_H
