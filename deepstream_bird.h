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

#ifndef __NVGSTDS_AUDIO_H__
#define __NVGSTDS_AUDIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>
#include <stdio.h>

#include "deepstream_app_version.h"
#include "deepstream_common.h"
#include "deepstream_config.h"
#include "deepstream_perf.h"
#include "deepstream_audio_classifier.h"
#include "deepstream_sinks.h"
#include "deepstream_sources.h"
#include "deepstream_streammux.h"


typedef struct _AppCtx AppCtx;

typedef void (*bbox_generated_callback)(AppCtx *appCtx, GstBuffer *buf,
                                        NvDsBatchMeta *batch_meta, guint index);

typedef struct
{
  guint index;
  gulong all_bbox_buffer_probe_id;
  gulong primary_bbox_buffer_probe_id;
  GstElement *bin;
  GstElement *tee;
  NvDsAudioClassifierBin audio_classifier_bin;
  NvDsSinkBin sink_bin;
  AppCtx *appCtx;
} NvDsInstanceBin;

typedef struct
{
  guint bus_id;
  GstElement *pipeline;
  NvDsSrcParentBin multi_src_bin;
  //NvDsSrcBin src_bin;
  NvDsInstanceBin instance_bin;
  NvDsInstanceBin common_elements;
  AppCtx *appCtx;
} NvDsPipeline;

typedef struct
{
  gboolean enable_perf_measurement;
  gint file_loop;
  gboolean source_list_enabled;
  guint total_num_sources;
  guint num_source_sub_bins;
  guint num_sink_sub_bins;
  guint perf_measurement_interval_sec;

  gchar **uri_list;
  NvDsSourceConfig multi_source_config[MAX_SOURCE_BINS];
  NvDsStreammuxConfig streammux_config;
  NvDsGieConfig audio_classifier_config;
  NvDsSinkSubBinConfig sink_bin_sub_bin_config[MAX_SINK_BINS];
} NvDsConfig;

struct _AppCtx
{
  gboolean version;
  gboolean cintr;
  gboolean seeking;
  gboolean quit;
  bbox_generated_callback bbox_generated_post_analytics_cb;
  gint audio_event_id;
  gint return_value;
  guint index;

  GMutex app_lock;
  GCond app_cond;

  NvDsPipeline pipeline;
  NvDsConfig config;
  NvDsAppPerfStructInt perf_struct;
};

typedef struct
{
    gint anomaly_count;
    gint meta_number;
    struct timespec timespec_first_frame;
    GstClockTime gst_ts_first_frame;
    GMutex lock_stream_rtcp_sr;
    guint32 id;
    gint frameCount;
    GstClockTime last_ntp_time;
} StreamSourceInfo;

typedef struct
{
    StreamSourceInfo streams[MAX_SOURCE_BINS];
} TestAppCtx;

/**
 * @brief  Create DS Anyalytics Pipeline per the appCtx
 *         configurations
 * @param  appCtx [IN/OUT] The application context
 *         providing the config info and where the
 *         pipeline resources are maintained
 * @param  perf_cb [IN]
 */
gboolean create_pipeline (AppCtx * appCtx, perf_callback perf_cb, bbox_generated_callback bgpa_cb);

gboolean pause_pipeline (AppCtx * appCtx);
gboolean resume_pipeline (AppCtx * appCtx);
gboolean seek_pipeline (AppCtx * appCtx, glong milliseconds, gboolean seek_is_relative);

void destroy_pipeline (AppCtx * appCtx);
void restart_pipeline (AppCtx * appCtx);

/**
 * Function to read properties from configuration file.
 *
 * @param[in] config pointer to @ref NvDsConfig
 * @param[in] cfg_file_path path of configuration file.
 *
 * @return true if parsed successfully.
 */
gboolean
parse_config_file (NvDsConfig * config, gchar * cfg_file_path);

#ifdef __cplusplus
}
#endif

#endif
