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
#include "deepstream_tracker.h"

GST_DEBUG_CATEGORY_EXTERN (NVDS_APP);

gboolean
create_tracking_bin (NvDsTrackerConfig *config, NvDsTrackerBin *bin)
{
  gboolean ret = FALSE;

  bin->bin = gst_bin_new ("tracking_bin");
  if (!bin->bin) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'tracking_bin'");
    goto done;
  }

  bin->tracker =
      gst_element_factory_make (NVDS_ELEM_TRACKER, "tracking_tracker");
  if (!bin->tracker) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'tracking_tracker'");
    goto done;
  }

  g_object_set (G_OBJECT (bin->tracker), "tracker-width", config->width,
      "tracker-height", config->height,
      "gpu-id", config->gpu_id,
      "ll-config-file", config->ll_config_file,
      "ll-lib-file", config->ll_lib_file,
      NULL);

  g_object_set (G_OBJECT (bin->tracker), "enable-batch-process",
        config->enable_batch_process, NULL);

  g_object_set (G_OBJECT (bin->tracker), "enable-past-frame",
        config->enable_past_frame, NULL);

  g_object_set (G_OBJECT (bin->tracker), "display-tracking-id",
        config->display_tracking_id, NULL);
  
  g_object_set (G_OBJECT (bin->tracker), "tracking-id-reset-mode",
        config->tracking_id_reset_mode, NULL);

  g_object_set (G_OBJECT (bin->tracker), "tracking-surface-type",
        config->tracking_surface_type, NULL);

  gst_bin_add_many (GST_BIN (bin->bin), bin->tracker, NULL);

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->tracker, "sink");

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->tracker, "src");

  ret = TRUE;

  GST_CAT_DEBUG (NVDS_APP, "Tracker bin created successfully");

done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }
  return ret;
}
