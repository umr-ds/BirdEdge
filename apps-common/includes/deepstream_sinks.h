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

#ifndef __NVGSTDS_SINKS_H__
#define __NVGSTDS_SINKS_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <gst/gst.h>

typedef enum
{
  NV_DS_SINK_FAKE = 1,
  NV_DS_SINK_RENDER_EGL,
  NV_DS_SINK_ENCODE_FILE,
  NV_DS_SINK_UDPSINK,
  NV_DS_SINK_RENDER_OVERLAY,
  NV_DS_SINK_MSG_CONV_BROKER,
} NvDsSinkType;

typedef enum
{
  NV_DS_CONTAINER_MP4 = 1,
  NV_DS_CONTAINER_MKV
} NvDsContainerType;

typedef enum
{
  NV_DS_ENCODER_H264 = 1,
  NV_DS_ENCODER_H265,
  NV_DS_ENCODER_MPEG4
} NvDsEncoderType;

typedef enum
{
  NV_DS_ENCODER_TYPE_HW,
  NV_DS_ENCODER_TYPE_SW
}NvDsEncHwSwType;

typedef struct
{
  NvDsSinkType type;
  NvDsContainerType container;
  NvDsEncoderType codec;
  NvDsEncHwSwType enc_type;
  gint bitrate;
  guint profile;
  gint sync;
  gchar *output_file_path;
  guint gpu_id;
  guint rtsp_port;
  guint udp_port;
  guint64 udp_buffer_size;
  guint iframeinterval;
} NvDsSinkEncoderConfig;

typedef struct
{
  NvDsSinkType type;
  gint width;
  gint height;
  gint sync;
  gboolean qos;
  gboolean qos_value_specified;
  guint gpu_id;
  guint nvbuf_memory_type;
  guint display_id;
  guint overlay_id;
  guint offset_x;
  guint offset_y;
} NvDsSinkRenderConfig;

typedef struct
{
  gboolean enable;
  /** MsgConv settings */
  gchar*    config_file_path;
  guint     conv_payload_type;
  gchar*    conv_msg2p_lib;
  guint     conv_comp_id;
  gchar*    debug_payload_dir;
  gboolean  multiple_payloads;
  gboolean  conv_msg2p_new_api;
  guint     conv_frame_interval;
  /** Broker settings */
  gchar*    proto_lib;
  gchar*    conn_str;
  gchar*    topic;
  gchar*    broker_config_file_path;
  guint     broker_comp_id;
  gboolean  disable_msgconv;
  gint sync;
  gboolean  new_api;
} NvDsSinkMsgConvBrokerConfig;

typedef struct
{
  gboolean enable;
  guint source_id;
  gboolean link_to_demux;
  NvDsSinkType type;
  gint sync;
  NvDsSinkEncoderConfig encoder_config;
  NvDsSinkRenderConfig render_config;
  NvDsSinkMsgConvBrokerConfig msg_conv_broker_config;
} NvDsSinkSubBinConfig;

typedef struct
{
  GstElement *bin;
  GstElement *queue;
  GstElement *transform;
  GstElement *cap_filter;
  GstElement *enc_caps_filter;
  GstElement *encoder;
  GstElement *codecparse;
  GstElement *mux;
  GstElement *sink;
  GstElement *rtppay;
  gulong sink_buffer_probe;
} NvDsSinkBinSubBin;

typedef struct
{
  GstElement *bin;
  GstElement *queue;
  GstElement *tee;

  gint num_bins;
  NvDsSinkBinSubBin sub_bins[MAX_SINK_BINS];
} NvDsSinkBin;

/**
 * Initialize @ref NvDsSinkBin. It creates and adds sink and
 * other elements needed for processing to the bin.
 * It also sets properties mentioned in the configuration file under
 * group @ref CONFIG_GROUP_SINK
 *
 * @param[in] num_sub_bins number of sink elements.
 * @param[in] config_array array of pointers of type @ref NvDsSinkSubBinConfig
 *            parsed from configuration file.
 * @param[in] bin pointer to @ref NvDsSinkBin to be filled.
 * @param[in] index id of source element.
 *
 * @return true if bin created successfully.
 */
gboolean create_sink_bin (guint num_sub_bins,
    NvDsSinkSubBinConfig *config_array, NvDsSinkBin *bin, guint index);

void destroy_sink_bin (void);
gboolean create_demux_sink_bin (guint num_sub_bins,
    NvDsSinkSubBinConfig *config_array, NvDsSinkBin *bin, guint index);

void set_rtsp_udp_port_num (guint rtsp_port_num, guint udp_port_num);

#ifdef __cplusplus
}
#endif

#endif
