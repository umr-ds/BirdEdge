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

#ifndef __NVGSTDS_C2D_MSG_UTIL_H__
#define __NVGSTDS_C2D_MSG_UTIL_H__

#include <glib.h>
#include "deepstream_c2d_msg.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
  NVDS_C2D_MSG_SR_START,
  NVDS_C2D_MSG_SR_STOP
} NvDsC2DMsgType;

typedef struct NvDsC2DMsg {
  NvDsC2DMsgType type;
  gpointer message;
  guint msgSize;
} NvDsC2DMsg;

typedef struct NvDsC2DMsgSR {
  gchar *sensorStr;
  gint startTime;
  guint duration;
} NvDsC2DMsgSR;

NvDsC2DMsg* nvds_c2d_parse_cloud_message (gpointer data, guint size);

void nvds_c2d_release_message (NvDsC2DMsg *msg);

gboolean nvds_c2d_parse_sensor (NvDsC2DContext *ctx, const gchar *file);

#ifdef __cplusplus
}
#endif
#endif
