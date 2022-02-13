/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION. All rights reserved.
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

#ifndef __NVGSTDS_DEWARPER_H__
#define __NVGSTDS_DEWARPER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>

typedef struct
{
  GstElement *bin;
  GstElement *queue;
  GstElement *src_queue;
  GstElement *nvvidconv;
  GstElement *cap_filter;
  GstElement *dewarper_caps_filter;
  GstElement *nvdewarper;
} NvDsDewarperBin;

typedef struct
{
  gboolean enable;
  guint gpu_id;
  guint num_out_buffers;
  guint dewarper_dump_frames;
  gchar *config_file;
  guint nvbuf_memory_type;
  guint source_id;
  guint num_surfaces_per_frame;
  guint num_batch_buffers;
} NvDsDewarperConfig;

gboolean create_dewarper_bin (NvDsDewarperConfig * config, NvDsDewarperBin * bin);

#ifdef __cplusplus
}
#endif

#endif
