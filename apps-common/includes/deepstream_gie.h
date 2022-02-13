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

#ifndef __NVGSTDS_GIE_H__
#define __NVGSTDS_GIE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>
#include "gstnvdsmeta.h"
#include "gstnvdsinfer.h"
#include "deepstream_config.h"

typedef enum
{
  NV_DS_GIE_PLUGIN_INFER = 0,
  NV_DS_GIE_PLUGIN_INFER_SERVER,
} NvDsGiePluginType;

typedef struct
{
  gboolean enable;

  gchar *config_file_path;

  gboolean input_tensor_meta;

  gboolean override_colors;

  gint operate_on_gie_id;
  gboolean is_operate_on_gie_id_set;
  gint operate_on_classes;

  gint num_operate_on_class_ids;
  gint *list_operate_on_class_ids;

  gboolean have_bg_color;
  NvOSD_ColorParams bbox_bg_color;
  NvOSD_ColorParams bbox_border_color;

  GHashTable *bbox_border_color_table;
  GHashTable *bbox_bg_color_table;

  guint batch_size;
  gboolean is_batch_size_set;

  guint interval;
  gboolean is_interval_set;
  guint unique_id;
  gboolean is_unique_id_set;
  guint gpu_id;
  gboolean is_gpu_id_set;
  guint nvbuf_memory_type;
  gchar *model_engine_file_path;

  gchar *audio_transform;
  guint frame_size;
  gboolean is_frame_size_set;
  guint hop_size;
  gboolean is_hop_size_set;
  guint input_audio_rate;

  gchar *label_file_path;
  guint n_labels;
  guint *n_label_outputs;
  gchar ***labels;

  gchar *raw_output_directory;
  gulong file_write_frame_num;

  gchar *tag;

  NvDsGiePluginType plugin_type;
} NvDsGieConfig;

#ifdef __cplusplus
}
#endif

#endif
