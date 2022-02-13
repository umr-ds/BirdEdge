/*
 * Copyright (c) 2018-2020, NVIDIA CORPORATION. All rights reserved.
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

#ifndef __NVGSTDS_TRACKER_H__
#define __NVGSTDS_TRACKER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>
#include <stdint.h>
#include "nvds_tracker_meta.h"

typedef struct
{
  gboolean enable;
  gint width;
  gint height;
  guint gpu_id;
  guint tracking_surf_type;
  gchar* ll_config_file;
  gchar* ll_lib_file;
  gboolean enable_batch_process;
  gboolean enable_past_frame;
  guint tracking_surface_type;
  gboolean display_tracking_id;
  guint tracking_id_reset_mode;
} NvDsTrackerConfig;

typedef struct
{
  GstElement *bin;
  GstElement *tracker;
} NvDsTrackerBin;

typedef uint64_t NvDsTrackerStreamId;

/**
 * Initialize @ref NvDsTrackerBin. It creates and adds tracker and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_TRACKER
 *
 * @param[in] config pointer of type @ref NvDsTrackerConfig
 *            parsed from configuration file.
 * @param[in] bin pointer of type @ref NvDsTrackerBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean
create_tracking_bin (NvDsTrackerConfig * config, NvDsTrackerBin * bin);

#ifdef __cplusplus
}
#endif

#endif
