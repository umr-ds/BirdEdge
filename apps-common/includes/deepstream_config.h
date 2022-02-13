/*
 * Copyright (c) 2018-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __NVGSTDS_CONFIG_H__
#define __NVGSTDS_CONFIG_H__

#ifdef __aarch64__
#define IS_TEGRA
#endif

#define MEMORY_FEATURES "memory:NVMM"

#ifdef IS_TEGRA
#define NVDS_ELEM_SRC_CAMERA_CSI "nvarguscamerasrc"
#else
#define NVDS_ELEM_SRC_CAMERA_CSI "videotestsrc"
#endif
#define NVDS_ELEM_SRC_CAMERA_V4L2 "v4l2src"
#define NVDS_ELEM_SRC_URI "uridecodebin"
#define NVDS_ELEM_SRC_MULTIFILE "multifilesrc"
#define NVDS_ELEM_SRC_ALSA "alsasrc"

#define NVDS_ELEM_DECODEBIN "decodebin"
#define NVDS_ELEM_WAVPARSE "wavparse"

#define NVDS_ELEM_QUEUE "queue"
#define NVDS_ELEM_CAPS_FILTER "capsfilter"
#define NVDS_ELEM_TEE "tee"

#define NVDS_ELEM_PREPROCESS "nvdspreprocess"
#define NVDS_ELEM_PGIE "nvinfer"
#define NVDS_ELEM_SGIE "nvinfer"
#define NVDS_ELEM_NVINFER "nvinfer"
#define NVDS_ELEM_INFER_SERVER "nvinferserver"
#define NVDS_ELEM_INFER_AUDIO "nvinferaudio"
#define NVDS_ELEM_TRACKER "nvtracker"

#define NVDS_ELEM_VIDEO_CONV "nvvideoconvert"
#define NVDS_ELEM_AUDIO_CONV "audioconvert"
#define NVDS_ELEM_AUDIO_RESAMPLER "audioresample"
#define NVDS_ELEM_STREAM_MUX "nvstreammux"
#define NVDS_ELEM_STREAM_DEMUX "nvstreamdemux"
#define NVDS_ELEM_TILER "nvmultistreamtiler"
#define NVDS_ELEM_OSD "nvdsosd"
#define NVDS_ELEM_DSANALYTICS_ELEMENT "nvdsanalytics"
#define NVDS_ELEM_DSEXAMPLE_ELEMENT "dsexample"

#define NVDS_ELEM_DEWARPER "nvdewarper"
#define NVDS_ELEM_SPOTANALYSIS "nvspot"
#define NVDS_ELEM_NVAISLE "nvaisle"
#define NVDS_ELEM_BBOXFILTER "nvbboxfilter"
#define NVDS_ELEM_MSG_CONV "nvmsgconv"
#define NVDS_ELEM_MSG_BROKER "nvmsgbroker"

#define NVDS_ELEM_SINK_FAKESINK "fakesink"
#define NVDS_ELEM_SINK_FILE "filesink"
#define NVDS_ELEM_SINK_EGL "nveglglessink"

#define NVDS_ELEM_SINK_OVERLAY "nvoverlaysink"
#define NVDS_ELEM_EGLTRANSFORM "nvegltransform"

#define NVDS_ELEM_MUX_MP4 "qtmux"
#define NVDS_ELEM_MKV "matroskamux"

#define NVDS_ELEM_ENC_H264_HW "nvv4l2h264enc"
#define NVDS_ELEM_ENC_H265_HW "nvv4l2h265enc"
#define NVDS_ELEM_ENC_MPEG4 "avenc_mpeg4"

#define NVDS_ELEM_ENC_H264_SW "x264enc"
#define NVDS_ELEM_ENC_H265_SW "x265enc"

#define MAX_SOURCE_BINS 1024
#define MAX_SINK_BINS (1024)
#define MAX_SECONDARY_GIE_BINS (16)
#define MAX_MESSAGE_CONSUMERS (16)

#endif
