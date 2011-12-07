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

#include <bmx/frame/Frame.h>
#include <bmx/BMXException.h>
#include <bmx/Logging.h>

using namespace std;
using namespace bmx;



Frame::Frame()
{
    position = NULL_FRAME_POSITION;
    track_position = NULL_FRAME_POSITION;
    ec_position = NULL_FRAME_POSITION;
    first_sample_offset = 0;
    num_samples = 0;
    temporal_reordering = false;
    temporal_offset = 0;
    key_frame_offset = 0;
    flags = 0;
    cp_file_position = 0;
    file_position = 0;
}

void Frame::CopyMetadataTo(Frame *to_frame) const
{
    to_frame->position             = position;
    to_frame->track_position       = track_position;
    to_frame->ec_position          = ec_position;
    to_frame->first_sample_offset  = first_sample_offset;
    to_frame->num_samples          = num_samples;
    to_frame->temporal_reordering  = temporal_reordering;
    to_frame->temporal_offset      = temporal_offset;
    to_frame->key_frame_offset     = key_frame_offset;
    to_frame->flags                = flags;
    to_frame->cp_file_position     = cp_file_position;
    to_frame->file_position        = file_position;
}



DefaultFrame::DefaultFrame()
: Frame()
{
}

DefaultFrame::~DefaultFrame()
{
}

uint32_t DefaultFrame::GetSize() const
{
    return mData.GetSize();
}

const unsigned char* DefaultFrame::GetBytes() const
{
    return mData.GetBytes();
}

void DefaultFrame::Grow(uint32_t min_size)
{
    mData.Grow(min_size);
}

uint32_t DefaultFrame::GetSizeAvailable() const
{
    return mData.GetSizeAvailable();
}

unsigned char* DefaultFrame::GetBytesAvailable() const
{
    return mData.GetBytesAvailable();
}

void DefaultFrame::SetSize(uint32_t size)
{
    mData.SetSize(size);
}

void DefaultFrame::IncrementSize(uint32_t inc)
{
    mData.IncrementSize(inc);
}



Frame* DefaultFrameFactory::CreateFrame()
{
    return new DefaultFrame();
}

