/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
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
#include "deepstream_dsanalytics.h"


// Create bin, add queue and the element, link all elements and ghost pads,
// Set the element properties from the parsed config
gboolean
create_dsanalytics_bin (NvDsDsAnalyticsConfig *config, NvDsDsAnalyticsBin *bin)
{
  gboolean ret = FALSE;

  bin->bin = gst_bin_new ("dsanalytics_bin");
  if (!bin->bin) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'dsanalytics_bin'");
    goto done;
  }

  bin->queue =
      gst_element_factory_make (NVDS_ELEM_QUEUE, "dsanalytics_queue");
  if (!bin->queue) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'dsanalytics_queue'");
    goto done;
  }

  bin->elem_dsanalytics =
      gst_element_factory_make (NVDS_ELEM_DSANALYTICS_ELEMENT, "dsanalytics0");
  if (!bin->elem_dsanalytics) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'dsanalytics0'");
    goto done;
  }

  gst_bin_add_many (GST_BIN (bin->bin), bin->queue,
      bin->elem_dsanalytics, NULL);

  NVGSTDS_LINK_ELEMENT (bin->queue, bin->elem_dsanalytics);

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->queue, "sink");

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->elem_dsanalytics, "src");

  g_object_set (G_OBJECT (bin->elem_dsanalytics),
      "config-file", config->config_file_path, NULL);

 ret = TRUE;

done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }

  return ret;
}
