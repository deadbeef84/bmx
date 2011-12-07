/*
 * Copyright (C) 2011, British Broadcasting Corporation
 * All Rights Reserved.
 *
 * Author: Philip de Nier
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the British Broadcasting Corporation nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstring>

#include <bmx/writer_helper/MPEG2LGWriterHelper.h>
#include <bmx/BMXException.h>
#include <bmx/Logging.h>

using namespace std;
using namespace bmx;


#define NULL_TEMPORAL_OFFSET        127



MPEG2LGWriterHelper::MPEG2LGWriterHelper()
{
    mPosition = 0;
    mPrevKeyFramePosition = -1;
    mKeyFramePosition = -1;
    mKeyFrameTemporalReference = 0;
    memset(mGOPTemporalOffsets, NULL_TEMPORAL_OFFSET, sizeof(mGOPTemporalOffsets));
    mGOPStartPosition = 0;
    mFirstGOP = true;
    mTemporalReference = 0;
    mHaveTemporalOffset = false;
    mTemporalOffset = NULL_TEMPORAL_OFFSET;
    mHavePrevTemporalOffset = false;
    mPrevTemporalOffset = NULL_TEMPORAL_OFFSET;
    mKeyFrameOffset = 0;
    mFlags = 0;
    mHaveGOPHeader = false;
    mSingleSequence = true;
    mBPictureCount = 0;
    mConstantBFrames = true;
    mLowDelay = true;
    mClosedGOP = true;
    mCurrentGOPClosed = false;
    mIdenticalGOP = true;
    mMaxGOP = 0;
    mMaxBPictureCount = 0;
    mBitRate = 0;
}

MPEG2LGWriterHelper::~MPEG2LGWriterHelper()
{
}

void MPEG2LGWriterHelper::ProcessFrame(const unsigned char *data, uint32_t size)
{
    mEssenceParser.ParseFrameInfo(data, size);

    MPEG2FrameType frame_type = mEssenceParser.GetFrameType();
    BMX_CHECK(frame_type != UNKNOWN_FRAME_TYPE);
    BMX_CHECK(mPosition > 0 || frame_type == I_FRAME); // require first frame to be an I-frame


    mHaveGOPHeader = mEssenceParser.HaveGOPHeader();

    if (mSingleSequence && mPosition > 0 && mEssenceParser.HaveSequenceHeader())
        mSingleSequence = false;

    if (mHaveGOPHeader) {
        if (!mEssenceParser.IsClosedGOP()) // descriptor closed GOP == true if all sequences are closed GOP
            mClosedGOP = false;
        mCurrentGOPClosed = mEssenceParser.IsClosedGOP();
    }

    if (frame_type == B_FRAME) {
        mBPictureCount++;
    } else if (mBPictureCount > 0) {
        if (mMaxBPictureCount > 0 && mBPictureCount != mMaxBPictureCount)
            mConstantBFrames = false;
        if (mBPictureCount > mMaxBPictureCount)
            mMaxBPictureCount = mBPictureCount;
        mBPictureCount = 0;
    }

    if (mHaveGOPHeader) {
        uint8_t gop_size = mPosition - mGOPStartPosition;
        if (gop_size > mMaxGOP)
            mMaxGOP = gop_size;
    }

    if (mEssenceParser.HaveSequenceHeader()) {
        if (mLowDelay && !mEssenceParser.IsLowDelay())
            mLowDelay = false;
        mBitRate = mEssenceParser.GetBitRate() * 400; // MPEG-2 bit rate is in 400 bits/second units
    }

    if (mIdenticalGOP) {
        if (mFirstGOP && mHaveGOPHeader && mPosition > 0)
            mFirstGOP = false;

        if (mFirstGOP) {
            mGOPStructure.push_back(frame_type);
            if (mGOPStructure.size() >= 256) { // eg. max gop size for xdcam is 15
                log_warn("Unexpected GOP size >= %zu\n", mGOPStructure.size());
                mIdenticalGOP = false;
            }
        } else {
            size_t pos_in_gop = (mHaveGOPHeader ? 0 : (size_t)(mPosition - mGOPStartPosition));
            if (pos_in_gop >= mGOPStructure.size() || mGOPStructure[pos_in_gop] != frame_type)
                mIdenticalGOP = false;
        }
    }

    mKeyFrameOffset = 0;
    if (frame_type != I_FRAME) {
        if (!mCurrentGOPClosed && mKeyFramePosition + mKeyFrameTemporalReference >= mPosition)
            mKeyFrameOffset = mPrevKeyFramePosition - mPosition;
        else
            mKeyFrameOffset = mKeyFramePosition - mPosition;
    }


    if (mHaveGOPHeader) {
        if (!CheckTemporalOffsetsComplete(0))
            log_warn("Incomplete MPEG-2 temporal offset data in index table\n");

        mGOPStartPosition = mPosition;
        memset(mGOPTemporalOffsets, NULL_TEMPORAL_OFFSET, sizeof(mGOPTemporalOffsets));
    }
    uint8_t gop_start_offset = mPosition - mGOPStartPosition;

    // temporal reference = display position for current frame
    mTemporalReference = mEssenceParser.GetTemporalReference();

    // temporal offset = offset to frame data required for displaying at the current position
    BMX_CHECK(mTemporalReference < sizeof(mGOPTemporalOffsets));
    mGOPTemporalOffsets[mTemporalReference] = gop_start_offset - mTemporalReference;

    mHaveTemporalOffset = (mGOPTemporalOffsets[gop_start_offset] != NULL_TEMPORAL_OFFSET);
    mTemporalOffset = mGOPTemporalOffsets[gop_start_offset];

    mHavePrevTemporalOffset = false;
    if (mTemporalReference < gop_start_offset) {
        // a temporal offset value for a previous position is now available
        mHavePrevTemporalOffset = true;
        mPrevTemporalOffset = mGOPTemporalOffsets[mTemporalReference];
    }

    mFlags = 0x00;
    if (mEssenceParser.HaveSequenceHeader())
        mFlags |= 0x40; // sequence header bit
    if (frame_type == I_FRAME) {
        if (mEssenceParser.HaveSequenceHeader() && mHaveGOPHeader && mEssenceParser.IsClosedGOP())
            mFlags |= 0x80; // reference frame bit
    } else if (frame_type == P_FRAME) {
        mFlags |= 0x22;
    } else {
        mFlags |= 0x33; // naive setting - assume forward and backward prediction
    }
    if (mKeyFrameOffset + mPosition < 0 || mTemporalOffset + mPosition < 0)
        mFlags |= 0x0b; // offsets out of range


    if (frame_type == I_FRAME) {
        mPrevKeyFramePosition = mKeyFramePosition;
        mKeyFramePosition = mPosition;
        mKeyFrameTemporalReference = mTemporalReference;
    }

    mPosition++;
}

bool MPEG2LGWriterHelper::CheckTemporalOffsetsComplete(int64_t end_offset)
{
    // check all temporal offsets are known (end_offset is <= 0)
    int64_t i;
    for (i = 0; i < mPosition - mGOPStartPosition + end_offset; i++) {
        if (mGOPTemporalOffsets[i] == NULL_TEMPORAL_OFFSET)
            return false;
    }

    return true;
}

