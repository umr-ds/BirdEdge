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

#include "deepstream_bird.h"
#include "deepstream_config_file_parser.h"
#include "nvds_version.h"
#include "nvdsmeta_schema.h"
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define APP_TITLE "DeepStream"

AppCtx *appCtx;
static guint cintr = FALSE;
static GMainLoop *main_loop = NULL;
static gchar **cfg_files = NULL;
static gchar **input_files = NULL;
static gboolean print_version = FALSE;
static gboolean print_dependencies_version = FALSE;
static gboolean quit = FALSE;
static gint return_value = 0;
static GMutex fps_lock;
static gdouble fps;
static gdouble fps_avg;
static gboolean playback_utc = TRUE;
static TestAppCtx *testAppCtx;

GST_DEBUG_CATEGORY(NVDS_APP);

GOptionEntry entries[] = {
    {"version", 'v', 0, G_OPTION_ARG_NONE, &print_version,
     "Print DeepStreamSDK version", NULL},
    {"version-all", 0, 0, G_OPTION_ARG_NONE, &print_dependencies_version,
     "Print DeepStreamSDK and dependencies version", NULL},
    {"cfg-file", 'c', 0, G_OPTION_ARG_FILENAME_ARRAY, &cfg_files,
     "Set the config file", NULL},
    {"playback-utc", 'p', 0, G_OPTION_ARG_INT, &playback_utc,
     "Playback utc; default=true (base UTC from file/rtsp URL); =false (base "
     "UTC from file-URL or RTCP Sender Report)",
     NULL},
    {"input-file", 'i', 0, G_OPTION_ARG_FILENAME_ARRAY, &input_files,
     "Set the input file", NULL},
    {NULL},
};

/**
 * Function to handle program interrupt signal.
 * It installs default handler after handling the interrupt.
 */
static void _intr_handler(int signum) {
    struct sigaction action;

    NVGSTDS_ERR_MSG_V("User Interrupted.. \n");

    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_DFL;

    sigaction(SIGINT, &action, NULL);

    cintr = TRUE;
}

/**
 * callback function to print the performance numbers of each stream.
 */
static void perf_cb(gpointer context, NvDsAppPerfStruct *str) {
    g_mutex_lock(&fps_lock);
    fps = str->fps[0];
    fps_avg = str->fps_avg[0];

    g_print("\n**PERF: ");
    g_print("FPS %d (Avg)\t", 0);
    g_print("\n");
    g_print("**PERF: ");
    g_print("%.2f (%.2f)\t", fps, fps_avg);
    g_print("\n");
    g_mutex_unlock(&fps_lock);
}

/**
 * Loop function to check the status of interrupts.
 * It comes out of loop if application got interrupted.
 */
static gboolean check_for_interrupt(gpointer data) {
    if (quit) {
        return FALSE;
    }

    if (cintr) {
        cintr = FALSE;

        quit = TRUE;
        g_main_loop_quit(main_loop);

        return FALSE;
    }
    return TRUE;
}

/*
 * Function to install custom handler for program interrupt signal.
 */
static void _intr_setup(void) {
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = _intr_handler;

    sigaction(SIGINT, &action, NULL);
}

static gboolean kbhit(void) {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}

/*
 * Function to enable / disable the canonical mode of terminal.
 * In non canonical mode input is available immediately (without the user
 * having to type a line-delimiter character).
 */
static void changemode(int dir) {
    static struct termios oldt, newt;

    if (dir == 1) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

static void print_runtime_commands(void) {
    g_print("\nRuntime commands:\n"
            "\th: Print this help\n"
            "\tq: Quit\n\n"
            "\tp: Pause\n"
            "\tr: Resume\n\n");
}

/**
 * Loop function to check keyboard inputs and status of each pipeline.
 */
static gboolean event_thread_func(gpointer arg) {
    gboolean ret = TRUE;

    if (appCtx->quit) {
        quit = TRUE;
        g_main_loop_quit(main_loop);
        return FALSE;
    }

    // Check for keyboard input
    if (!kbhit()) {
        // continue;
        return TRUE;
    }
    int c = fgetc(stdin);
    g_print("\n");

    switch (c) {
    case 'h':
        print_runtime_commands();
        break;
    case 'p':
        pause_pipeline(appCtx);
        break;
    case 'r':
        resume_pipeline(appCtx);
        break;
    case 'q':
        quit = TRUE;
        g_main_loop_quit(main_loop);
        ret = FALSE;
        break;
    default:
        break;
    }
    return ret;
}

static GstClockTime generate_ts_rfc3339_from_ts(char *buf, int buf_size,
                                                GstClockTime ts) {
    time_t tloc;
    struct tm tm_log;
    char strmsec[6];
    int ms;
    GstClockTime ts_generated;
    /** ts itself is UTC Time in ns */
    struct timespec timespec_current;
    GST_TIME_TO_TIMESPEC(ts, timespec_current);
    memcpy(&tloc, (void *)(&timespec_current.tv_sec), sizeof(time_t));
    ms = timespec_current.tv_nsec / 1000000;
    ts_generated = ts;

    gmtime_r(&tloc, &tm_log);
    strftime(buf, buf_size, "%Y-%m-%dT%H:%M:%S", &tm_log);
    g_snprintf(strmsec, sizeof(strmsec), ".%.3dZ", ms);
    strncat(buf, strmsec, buf_size);
    return ts_generated;
}

static void generate_event_msg_meta(gpointer data, gint class_id,
                                    NvDsAudioFrameMeta *frame_meta) {
    NvDsEventMsgMeta *meta = (NvDsEventMsgMeta *)data;
    meta->type = class_id;
    meta->confidence = frame_meta->confidence;
    meta->ts = (gchar *)g_malloc0(MAX_LABEL_SIZE);
    generate_ts_rfc3339_from_ts(meta->ts, MAX_LABEL_SIZE,
                                frame_meta->ntp_timestamp);
    meta->objectId = g_strdup(frame_meta->class_label);
    meta->sensorId = frame_meta->source_id;
    meta->placeId = frame_meta->source_id;
}

static void meta_free_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *)data;
    NvDsEventMsgMeta *srcMeta = (NvDsEventMsgMeta *)user_meta->user_meta_data;
    user_meta->user_meta_data = NULL;

    if (srcMeta->ts) {
        g_free(srcMeta->ts);
    }

    if (srcMeta->objSignature.size > 0) {
        g_free(srcMeta->objSignature.signature);
        srcMeta->objSignature.size = 0;
    }

    if (srcMeta->objectId) {
        g_free(srcMeta->objectId);
    }

    if (srcMeta->sensorStr) {
        g_free(srcMeta->sensorStr);
    }

    if (srcMeta->extMsgSize > 0) {
        g_free(srcMeta->extMsg);
        srcMeta->extMsg = NULL;
        srcMeta->extMsgSize = 0;
    }
    g_free(srcMeta);
}

static gpointer meta_copy_func(gpointer data, gpointer user_data) {
    NvDsUserMeta *user_meta = (NvDsUserMeta *)data;
    NvDsEventMsgMeta *srcMeta = (NvDsEventMsgMeta *)user_meta->user_meta_data;
    NvDsEventMsgMeta *dstMeta = NULL;

    dstMeta = g_memdup(srcMeta, sizeof(NvDsEventMsgMeta));

    if (srcMeta->ts)
        dstMeta->ts = g_strdup(srcMeta->ts);

    if (srcMeta->objSignature.size > 0) {
        dstMeta->objSignature.signature = g_memdup(
            srcMeta->objSignature.signature, srcMeta->objSignature.size);
        dstMeta->objSignature.size = srcMeta->objSignature.size;
    }

    if (srcMeta->objectId) {
        dstMeta->objectId = g_strdup(srcMeta->objectId);
    }

    if (srcMeta->sensorStr) {
        dstMeta->sensorStr = g_strdup(srcMeta->sensorStr);
    }

    return dstMeta;
}

/**
 * Callback function to be called once all inferences (Primary + Secondary)
 * are done. This is opportunity to modify content of the metadata.
 * e.g. Here Person is being replaced with Man/Woman and corresponding counts
 * are being maintained. It should be modified according to network classes
 * or can be removed altogether if not required.
 */
static void print_predictions(AppCtx *appCtx, GstBuffer *buf,
                              NvDsBatchMeta *batch_meta, guint index) {
    guint32 stream_id = 0;

    for (NvDsMetaList *l_frame = batch_meta->frame_meta_list; l_frame != NULL;
         l_frame = l_frame->next) {
        NvDsAudioFrameMeta *frame_meta = l_frame->data;

        g_print("{"
                "\"frame_num\": %d, "
                "\"timestamp\": %ld, "
                "\"label\": \"%s\", "
                "\"source_id\": %d, "
                "\"confidence\": %f"
                "}\n",
                frame_meta->frame_num, frame_meta->ntp_timestamp,
                frame_meta->class_label, frame_meta->source_id,
                frame_meta->confidence);

        // if (!strcmp(frame_meta->class_label, "00_background")) {
        //     g_print("### frame_num:[%d] ntp_timestamp:[%ld] label:[%s] "
        //             "source_id:[%d] confidence:[%f]\n",
        //             frame_meta->frame_num, frame_meta->ntp_timestamp, "",
        //             frame_meta->source_id, 0.0);

        // } else {
        //     g_print("### frame_num:[%d] ntp_timestamp:[%ld] label:[%s] "
        //             "source_id:[%d] confidence:[%f]\n",
        //             frame_meta->frame_num, frame_meta->ntp_timestamp,
        //             frame_meta->class_label, frame_meta->source_id,
        //             frame_meta->confidence);
        // }
        stream_id = frame_meta->source_id;
        GstClockTime buf_ntp_time = 0;
        if (playback_utc == FALSE) {
            /** Calculate the buffer-NTP-time
             * derived from this stream's RTCP Sender Report here:
             */
            StreamSourceInfo *src_stream = &testAppCtx->streams[stream_id];
            buf_ntp_time = frame_meta->ntp_timestamp;

            if (buf_ntp_time < src_stream->last_ntp_time) {
                NVGSTDS_WARN_MSG_V(
                    "Source %d: NTP timestamps are backward in time."
                    " Current: %lu previous: %lu",
                    stream_id, buf_ntp_time, src_stream->last_ntp_time);
            }
            src_stream->last_ntp_time = buf_ntp_time;
        }
        testAppCtx->streams[stream_id].frameCount++;
    }
}

int main(int argc, char *argv[]) {
    testAppCtx = (TestAppCtx *)g_malloc0(sizeof(TestAppCtx));
    GOptionContext *ctx = NULL;
    GOptionGroup *group = NULL;
    GError *error = NULL;
    guint i = 0;

    /** deepstream-audio app would work only with the new streammux */
    putenv("USE_NEW_NVSTREAMMUX=yes");

    ctx = g_option_context_new("Nvidia DeepStream Demo");
    group = g_option_group_new("abc", NULL, NULL, NULL, NULL);
    g_option_group_add_entries(group, entries);

    g_option_context_set_main_group(ctx, group);
    g_option_context_add_group(ctx, gst_init_get_option_group());

    GST_DEBUG_CATEGORY_INIT(NVDS_APP, "NVDS_APP", 0, NULL);

    if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
        NVGSTDS_ERR_MSG_V("%s", error->message);
        return -1;
    }

    if (print_version) {
        g_print("deepstream-app version %d.%d.%d\n", NVDS_APP_VERSION_MAJOR,
                NVDS_APP_VERSION_MINOR, NVDS_APP_VERSION_MICRO);
        nvds_version_print();
        return 0;
    }

    if (print_dependencies_version) {
        g_print("deepstream-audio version %d.%d.%d\n", NVDS_APP_VERSION_MAJOR,
                NVDS_APP_VERSION_MINOR, NVDS_APP_VERSION_MICRO);
        nvds_version_print();
        nvds_dependencies_version_print();
        return 0;
    }

    if (!cfg_files) {
        NVGSTDS_ERR_MSG_V("Specify config file with -c option");
        return_value = -1;
        goto done;
    }

    appCtx = g_malloc0(sizeof(AppCtx));
    appCtx->audio_event_id = -1;
    appCtx->index = i;

#if 0
  if (input_files) {
    appCtx->config.source_config.uri =
        g_strdup_printf ("file://%s", input_files[0]);
    g_free (input_files[0]);
  }
#endif

    if (!parse_config_file(&appCtx->config, cfg_files[0])) {
        NVGSTDS_ERR_MSG_V("Failed to parse config file '%s'", cfg_files[0]);
        appCtx->return_value = -1;
        goto done;
    }

    if (!create_pipeline(appCtx, perf_cb, print_predictions)) {
        NVGSTDS_ERR_MSG_V("Failed to create pipeline");
        return_value = -1;
        goto done;
    }

    main_loop = g_main_loop_new(NULL, FALSE);

    _intr_setup();
    g_timeout_add(400, check_for_interrupt, NULL);

    if (gst_element_set_state(appCtx->pipeline.pipeline, GST_STATE_PAUSED) ==
        GST_STATE_CHANGE_FAILURE) {
        NVGSTDS_ERR_MSG_V("Failed to set pipeline to PAUSED");
        return_value = -1;
        goto done;
    }

    if (gst_element_set_state(appCtx->pipeline.pipeline, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {

        g_print("\ncan't set pipeline to playing state.\n");
        return_value = -1;
        goto done;
    }

    // print_runtime_commands ();

    changemode(1);

    g_timeout_add(40, event_thread_func, NULL);
    g_main_loop_run(main_loop);

    changemode(0);

done:

    g_print("Quitting\n");
    if (appCtx) {
        if (appCtx->return_value == -1)
            return_value = -1;
        destroy_pipeline(appCtx);
        g_free(appCtx);
    }

    if (main_loop) {
        g_main_loop_unref(main_loop);
    }

    if (ctx) {
        g_option_context_free(ctx);
    }

    if (return_value == 0) {
        g_print("App run successful\n");
    } else {
        g_print("App run failed\n");
    }

    gst_deinit();
    g_free(testAppCtx);
    return return_value;
}
