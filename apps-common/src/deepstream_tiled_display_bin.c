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

#include "deepstream_common.h"
#include "deepstream_tiled_display.h"

gboolean
create_tiled_display_bin (NvDsTiledDisplayConfig *config, NvDsTiledDisplayBin *bin)
{
  gboolean ret = FALSE;

  bin->bin = gst_bin_new ("tiled_display_bin");
  if (!bin->bin) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'tiled_display_bin'");
    goto done;
  }

  bin->queue = gst_element_factory_make (NVDS_ELEM_QUEUE, "tiled_display_queue");
  if (!bin->queue) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'tiled_display_queue'");
    goto done;
  }

  if ((config->enable == 1) || (config->enable == 2))
    bin->tiler = gst_element_factory_make (NVDS_ELEM_TILER, "tiled_display_tiler");
  else if (config->enable == 3)
    bin->tiler = gst_element_factory_make ("identity", "tiled_display_identity");

  if (!bin->tiler) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'tiled_display_tiler'");
    goto done;
  }

  if (config->width)
    g_object_set (G_OBJECT (bin->tiler), "width", config->width, NULL);

  if (config->height)
    g_object_set (G_OBJECT (bin->tiler), "height", config->height, NULL);

  if (config->rows)
    g_object_set (G_OBJECT (bin->tiler), "rows", config->rows, NULL);

  if (config->columns)
    g_object_set (G_OBJECT (bin->tiler), "columns", config->columns, NULL);

  g_object_set (G_OBJECT (bin->tiler), "gpu-id", config->gpu_id,
       "nvbuf-memory-type", config->nvbuf_memory_type, NULL);

  gst_bin_add_many (GST_BIN (bin->bin), bin->queue, bin->tiler, NULL);

  NVGSTDS_LINK_ELEMENT (bin->queue, bin->tiler);

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->queue, "sink");

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->tiler, "src");

  ret = TRUE;
done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }
  return ret;
}
