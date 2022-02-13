/*
 * Copyright (c) 2018-2021, NVIDIA CORPORATION. All rights reserved.
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

#ifndef __NVGSTDS_SOURCES_H__
#define __NVGSTDS_SOURCES_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>
#include "deepstream_dewarper.h"
#include <sys/time.h>

typedef enum
{
  NV_DS_SOURCE_CAMERA_V4L2 = 1,
  NV_DS_SOURCE_URI,
  NV_DS_SOURCE_URI_MULTIPLE,
  NV_DS_SOURCE_RTSP,
  NV_DS_SOURCE_CAMERA_CSI,
  NV_DS_SOURCE_AUDIO_WAV,
  NV_DS_SOURCE_AUDIO_URI,
  NV_DS_SOURCE_ALSA_SRC,
} NvDsSourceType;

typedef struct
{
  NvDsSourceType type;
  gboolean enable;
  gboolean loop;
  gboolean live_source;
  gboolean Intra_decode;
  guint smart_record;
  gint source_width;
  gint source_height;
  gint source_fps_n;
  gint source_fps_d;
  gint camera_csi_sensor_id;
  gint camera_v4l2_dev_node;
  gchar *uri;
  gchar *dir_path;
  gchar *file_prefix;
  gint latency;
  guint smart_rec_cache_size;
  guint smart_rec_container;
  guint smart_rec_def_duration;
  guint smart_rec_duration;
  guint smart_rec_start_time;
  guint smart_rec_interval;
  guint num_sources;
  guint gpu_id;
  guint camera_id;
  guint source_id;
  guint select_rtp_protocol;
  guint num_decode_surfaces;
  guint num_extra_surfaces;
  guint nvbuf_memory_type;
  guint cuda_memory_type;
  NvDsDewarperConfig dewarper_config;
  guint drop_frame_interval;
  gint rtsp_reconnect_interval_sec;
  guint rtsp_reconnect_attempts;
  /** Desired input audio rate to nvinferaudio from PGIE config;
   * This config shall be copied over from NvDsGieConfig
   * at create_multi_source_bin()*/
  guint input_audio_rate;
  /** ALSA device, as defined in an asound configuration file */
  gchar* alsa_device;
} NvDsSourceConfig;

typedef struct NvDsSrcParentBin NvDsSrcParentBin;

typedef struct
{
  GstElement *bin;
  GstElement *src_elem;
  GstElement *cap_filter;
  GstElement *cap_filter1;
  GstElement *depay;
  GstElement *parser;
  GstElement *enc_que;
  GstElement *dec_que;
  GstElement *decodebin;
  GstElement *enc_filter;
  GstElement *encbin_que;
  GstElement *tee;
  GstElement *tee_rtsp_pre_decode;
  GstElement *tee_rtsp_post_decode;
  GstElement *fakesink_queue;
  GstElement *fakesink;
  GstElement *nvvidconv;
  GstElement *audio_converter;
  GstElement *audio_resample;

  gboolean do_record;
  guint64 pre_event_rec;
  GMutex bin_lock;
  guint bin_id;
  gint rtsp_reconnect_interval_sec;
  gint rtsp_reconnect_attempts;
  gint num_rtsp_reconnects;
  gboolean have_eos;
  struct timeval last_buffer_time;
  struct timeval last_reconnect_time;
  gulong src_buffer_probe;
  gulong rtspsrc_monitor_probe;
  gpointer bbox_meta;
  GstBuffer *inbuf;
  gchar *location;
  gchar *file;
  gchar *direction;
  gint latency;
  gboolean got_key_frame;
  gboolean eos_done;
  gboolean reset_done;
  gboolean live_source;
  gboolean reconfiguring;
  gboolean async_state_watch_running;
  NvDsDewarperBin dewarper_bin;
  gulong probe_id;
  guint64 accumulated_base;
  guint64 prev_accumulated_base;
  guint source_id;
  NvDsSourceConfig *config;
  NvDsSrcParentBin *parent_bin;
  gpointer recordCtx;
} NvDsSrcBin;

struct NvDsSrcParentBin
{
  GstElement *bin;
  GstElement *streammux;
  GThread *reset_thread;
  NvDsSrcBin sub_bins[MAX_SOURCE_BINS];
  guint num_bins;
  guint num_fr_on;
  gboolean live_source;
  gulong nvstreammux_eosmonitor_probe;
};


gboolean create_source_bin (NvDsSourceConfig *config, NvDsSrcBin *bin);
gboolean create_audio_source_bin (NvDsSourceConfig *config, NvDsSrcBin *bin);

/**
 * Initialize @ref NvDsSrcParentBin. It creates and adds source and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_SOURCE
 *
 * @param[in] num_sub_bins number of source elements.
 * @param[in] configs array of pointers of type @ref NvDsSourceConfig
 *            parsed from configuration file.
 * @param[in] bin pointer to @ref NvDsSrcParentBin to be filled.
 *
 * @return true if bin created successfully.
 */
gboolean
create_multi_source_bin (guint num_sub_bins, NvDsSourceConfig *configs,
                         NvDsSrcParentBin *bin);

gboolean reset_source_pipeline (gpointer data);
gboolean set_source_to_playing (gpointer data);
gpointer reset_encodebin (gpointer data);
void destroy_smart_record_bin (gpointer data);
#ifdef __cplusplus
}
#endif

#endif
