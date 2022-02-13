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

#include <linux/limits.h> /* For PATH_MAX */
#include <stdio.h>
#include <string.h>
#include "deepstream_common.h"
#include "deepstream_primary_gie.h"

static void
write_infer_output_to_file (GstBuffer *buf,
    NvDsInferNetworkInfo *network_info,  NvDsInferLayerInfo *layers_info,
    guint num_layers, guint batch_size, gpointer user_data)
{
  NvDsGieConfig *config = (NvDsGieConfig *) user_data;
  guint i;

  /* "gst_buffer_get_nvstream_meta()" can be called on the GstBuffer to get more
   * information about the buffer.*/

  for (i = 0; i < num_layers; i++) {
    NvDsInferLayerInfo *info = &layers_info[i];
    guint element_size = 0;
    guint j;
    FILE *file;
    gchar file_name[PATH_MAX];
    gchar layer_name[128];

    switch (info->dataType) {
      case FLOAT: element_size = 4; break;
      case HALF: element_size = 2; break;
      case INT32: element_size = 4; break;
      case INT8: element_size = 1; break;
    }

    g_strlcpy (layer_name, info->layerName, 128);
    for (j = 0; layer_name[j] != '\0'; j++) {
      layer_name[j] = (layer_name[j] == '/') ? '_' : layer_name[j];
    }

    g_snprintf (file_name, PATH_MAX,
        "%s/%s_batch%010lu_batchsize%02d.bin",
        config->raw_output_directory, layer_name,
        config->file_write_frame_num, batch_size);
    file_name[PATH_MAX -  1] = '\0';

    file = fopen (file_name, "w");
    if (!file) {
      g_printerr ("Could not open file '%s' for writing:%s\n",
          file_name, strerror(errno));
      continue;
    }
    fwrite (info->buffer, element_size, info->inferDims.numElements * batch_size, file);
    fclose (file);
  }
  config->file_write_frame_num++;
}

gboolean
create_primary_gie_bin (NvDsGieConfig *config, NvDsPrimaryGieBin *bin)
{
  gboolean ret = FALSE;
  gst_nvinfer_raw_output_generated_callback out_callback =
        write_infer_output_to_file;

  bin->bin = gst_bin_new ("primary_gie_bin");
  if (!bin->bin) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'primary_gie_bin'");
    goto done;
  }

  bin->nvvidconv =
      gst_element_factory_make (NVDS_ELEM_VIDEO_CONV, "primary_gie_conv");
  if (!bin->nvvidconv) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'primary_gie_conv'");
    goto done;
  }

  bin->queue = gst_element_factory_make (NVDS_ELEM_QUEUE, "primary_gie_queue");
  if (!bin->queue) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'primary_gie_queue'");
    goto done;
  }

  switch (config->plugin_type) {
    case NV_DS_GIE_PLUGIN_INFER:
      bin->primary_gie =
      gst_element_factory_make (NVDS_ELEM_PGIE, "primary_gie");
      break;
    case NV_DS_GIE_PLUGIN_INFER_SERVER:
      bin->primary_gie =
      gst_element_factory_make (NVDS_ELEM_INFER_SERVER, "primary_gie");
      break;
    default:
      NVGSTDS_ERR_MSG_V ("Failed to create 'primary_gie' "
        "on unknown plugin_type");
      goto done;
  }

  if (!bin->primary_gie) {
    NVGSTDS_ERR_MSG_V ("Failed to create 'primary_gie'");
    goto done;
  }

  g_object_set (G_OBJECT (bin->primary_gie),
      "config-file-path", GET_FILE_PATH (config->config_file_path),
      "process-mode", 1, NULL);

  g_object_set (G_OBJECT (bin->primary_gie),
    "input-tensor-meta", config->input_tensor_meta, NULL);

  if (config->is_batch_size_set)
    g_object_set (G_OBJECT (bin->primary_gie),
        "batch-size", config->batch_size, NULL);

  if (config->is_interval_set)
    g_object_set (G_OBJECT (bin->primary_gie),
        "interval", config->interval, NULL);

  if (config->is_unique_id_set)
    g_object_set (G_OBJECT (bin->primary_gie),
        "unique-id", config->unique_id, NULL);

  if (config->is_gpu_id_set &&
      NV_DS_GIE_PLUGIN_INFER_SERVER == config->plugin_type) {
    NVGSTDS_INFO_MSG_V ("gpu-id: %u in primary-gie group is ignored, "
        "only accept in nvinferserver's config", config->gpu_id);
  }

  if (config->raw_output_directory) {
    g_object_set (G_OBJECT (bin->primary_gie),
        "raw-output-generated-callback", out_callback,
        "raw-output-generated-userdata", config,
        NULL);
  }

  if (NV_DS_GIE_PLUGIN_INFER == config->plugin_type) {
    if (config->is_gpu_id_set)
      g_object_set (G_OBJECT (bin->primary_gie),
          "gpu-id", config->gpu_id, NULL);

    if (config->model_engine_file_path) {
      g_object_set (G_OBJECT (bin->primary_gie), "model-engine-file",
          GET_FILE_PATH (config->model_engine_file_path), NULL);
    }
  }

  g_object_set (G_OBJECT (bin->nvvidconv), "gpu-id", config->gpu_id, NULL);
  g_object_set (G_OBJECT (bin->nvvidconv), "nvbuf-memory-type",
      config->nvbuf_memory_type, NULL);

  gst_bin_add_many (GST_BIN (bin->bin), bin->queue,
      bin->nvvidconv, bin->primary_gie, NULL);

  NVGSTDS_LINK_ELEMENT (bin->queue, bin->nvvidconv);

  NVGSTDS_LINK_ELEMENT (bin->nvvidconv, bin->primary_gie);

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->primary_gie, "src");

  NVGSTDS_BIN_ADD_GHOST_PAD (bin->bin, bin->queue, "sink");

  ret = TRUE;
done:
  if (!ret) {
    NVGSTDS_ERR_MSG_V ("%s failed", __func__);
  }
  return ret;
}
