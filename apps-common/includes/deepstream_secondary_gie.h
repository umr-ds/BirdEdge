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

#ifndef __NVGSTDS_SECONDARY_GIE_H__
#define __NVGSTDS_SECONDARY_GIE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "deepstream_gie.h"

typedef struct
{
  GstElement *queue;
  GstElement *secondary_gie;
  GstElement *tee;
  GstElement *sink;
  gboolean create;
  guint num_children;
  gint parent_index;
} NvDsSecondaryGieBinSubBin;

typedef struct
{
  GstElement *bin;
  GstElement *tee;
  GstElement *queue;
  gulong wait_for_sgie_process_buf_probe_id;
  gboolean stop;
  gboolean flush;
  NvDsSecondaryGieBinSubBin sub_bins[MAX_SECONDARY_GIE_BINS];
  GMutex wait_lock;
  GCond wait_cond;
} NvDsSecondaryGieBin;

/**
 * Initialize @ref NvDsSecondaryGieBin. It creates and adds secondary infer and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_SECONDARY_GIE
 *
 * @param[in] num_secondary_gie number of secondary infers.
 * @param[in] primary_gie_unique_id Unique id of primary infer to work on.
 * @param[in] config_array array of pointers of type @ref NvDsGieConfig
 *            parsed from configuration file.
 * @param[in] bin pointer to @ref NvDsSecondaryGieBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean create_secondary_gie_bin (guint num_secondary_gie,
    guint primary_gie_unique_id, NvDsGieConfig *config_array,
    NvDsSecondaryGieBin *bin);

/**
 * Release the resources.
 */
void destroy_secondary_gie_bin (NvDsSecondaryGieBin *bin);

#ifdef __cplusplus
}
#endif

#endif
