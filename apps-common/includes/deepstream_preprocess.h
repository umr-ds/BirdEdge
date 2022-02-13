/*
 * Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
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

#ifndef __NVGSTDS_PREPROCESS_H__
#define __NVGSTDS_PREPROCESS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>

typedef struct
{
  /** create a bin for the element only if enabled */
  gboolean enable;
  /** config file path having properties for preprocess */
  gchar *config_file_path;
} NvDsPreProcessConfig;

typedef struct
{
  GstElement *bin;
  GstElement *queue;
  GstElement *preprocess;
} NvDsPreProcessBin;

/**
 * Initialize @ref NvDsPreProcessBin. It creates and adds preprocess and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_PREPROCESS
 *
 * @param[in] config pointer to infer @ref NvDsPreProcessConfig parsed from
 *            configuration file.
 * @param[in] bin pointer to @ref NvDsPreProcessBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean create_preprocess_bin (NvDsPreProcessConfig *config,
    NvDsPreProcessBin *bin);

#ifdef __cplusplus
}
#endif

#endif