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

#ifndef __NVGSTDS_TILED_DISPLAY_H__
#define __NVGSTDS_TILED_DISPLAY_H__

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
  GstElement *tiler;
} NvDsTiledDisplayBin;

typedef enum
{
  NV_DS_TILED_DISPLAY_DISABLE = 0,
  NV_DS_TILED_DISPLAY_ENABLE = 1,
  /** When user sets tiler group enable=2,
   * all sinks with the key: link-only-to-demux=1
   * shall be linked to demuxer's src_[source_id] pad
   * where source_id is the key set in this
   * corresponding [sink] group
   */
  NV_DS_TILED_DISPLAY_ENABLE_WITH_PARALLEL_DEMUX = 2
} NvDsTiledDisplayEnable;

typedef struct
{
  NvDsTiledDisplayEnable enable;
  guint rows;
  guint columns;
  guint width;
  guint height;
  guint gpu_id;
  guint nvbuf_memory_type;
} NvDsTiledDisplayConfig;

/**
 * Initialize @ref NvDsTiledDisplayBin. It creates and adds tiling and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_TILED_DISPLAY
 *
 * @param[in] config pointer of type @ref NvDsTiledDisplayConfig
 *            parsed from configuration file.
 * @param[in] bin pointer to @ref NvDsTiledDisplayBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean
create_tiled_display_bin (NvDsTiledDisplayConfig * config,
                          NvDsTiledDisplayBin * bin);

#ifdef __cplusplus
}
#endif

#endif
