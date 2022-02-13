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


#ifndef __NVGSTDS_C2D_MSG_H__
#define __NVGSTDS_C2D_MSG_H__

#include <gst/gst.h>
#include "nvmsgbroker.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct NvDsC2DContext {
  gpointer libHandle;
  gchar *protoLib;
  gchar *connStr;
  gchar *configFile;
  gpointer uData;
  GHashTable *hashMap;
  NvMsgBrokerClientHandle connHandle;
  nv_msgbroker_subscribe_cb_t subscribeCb;
} NvDsC2DContext;

typedef struct NvDsMsgConsumerConfig {
  gboolean enable;
  gchar *proto_lib;
  gchar *conn_str;
  gchar *config_file_path;
  GPtrArray *topicList;
  gchar *sensor_list_file;
} NvDsMsgConsumerConfig;

NvDsC2DContext*
start_cloud_to_device_messaging (NvDsMsgConsumerConfig *config,
                                 nv_msgbroker_subscribe_cb_t cb,
                                 void *uData);
gboolean stop_cloud_to_device_messaging (NvDsC2DContext* uCtx);

#ifdef __cplusplus
}
#endif
#endif
