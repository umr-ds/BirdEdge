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

#ifndef __NVGSTDS_OSD_H__
#define __NVGSTDS_OSD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>

#include "nvll_osd_struct.h"

typedef struct
{
  GstElement *bin;
  GstElement *queue;
  GstElement *nvvidconv;
  GstElement *conv_queue;
  GstElement *cap_filter;
  GstElement *nvosd;
} NvDsOSDBin;

typedef struct
{
  gboolean enable;
  gboolean text_has_bg;
  gboolean enable_clock;
  gboolean draw_text;
  gboolean draw_bbox;
  gboolean draw_mask;
  gint text_size;
  gint border_width;
  gint clock_text_size;
  gint clock_x_offset;
  gint clock_y_offset;
  guint gpu_id;
  guint nvbuf_memory_type; /* For nvvidconv */
  guint num_out_buffers;
  gchar *font;
  gchar *hw_blend_color_attr;
  NvOSD_Mode mode;
  NvOSD_ColorParams clock_color;
  NvOSD_ColorParams text_color;
  NvOSD_ColorParams text_bg_color;
} NvDsOSDConfig;

/**
 * Initialize @ref NvDsOSDBin. It creates and adds OSD and other elements
 * needed for processing to the bin. It also sets properties mentioned
 * in the configuration file under group @ref CONFIG_GROUP_OSD
 *
 * @param[in] config pointer to OSD @ref NvDsOSDConfig parsed from config file.
 * @param[in] bin pointer to @ref NvDsOSDBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean create_osd_bin (NvDsOSDConfig *config, NvDsOSDBin *bin);

#ifdef __cplusplus
}
#endif

#endif
