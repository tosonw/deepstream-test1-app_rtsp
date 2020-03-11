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

#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include "gstnvdsmeta.h"

#define MAX_DISPLAY_LEN 64

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 4000000

gint frame_number = 0;
gchar pgie_classes_str[4][32] = {"Vehicle", "TwoWheeler", "Person",
                                 "Roadsign"
};

/* osd_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
 * and update params for drawing rectangle, object information etc. */
//提取元数据从OSD slink绘制矩形框和物体信息等。
//static GstPadProbeReturn
//osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
//                          gpointer u_data) {
//    GstBuffer *buf = (GstBuffer *) info->data;
//    guint num_rects = 0;
//    NvDsObjectMeta *obj_meta = NULL;
//    guint vehicle_count = 0;
//    guint person_count = 0;
//    NvDsMetaList *l_frame = NULL;
//    NvDsMetaList *l_obj = NULL;
//    NvDsDisplayMeta *display_meta = NULL;
//
//    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);//包含多帧
//
//    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
//         l_frame = l_frame->next) {
//        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);//帧数据
//        int offset = 0;
//        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
//             l_obj = l_obj->next) {
//            obj_meta = (NvDsObjectMeta *) (l_obj->data);
//            if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
//                vehicle_count++;
//                num_rects++;
//            }
//            if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
//                person_count++;
//                num_rects++;
//            }
//        }//for循环统计该帧的车辆和行人
//        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
//        NvOSD_TextParams *txt_params = &display_meta->text_params[0];
//        display_meta->num_labels = 1;
//        txt_params->display_text = g_malloc0(MAX_DISPLAY_LEN);
//        offset = snprintf(txt_params->display_text, MAX_DISPLAY_LEN, "Person = %d ", person_count);
//        offset = snprintf(txt_params->display_text + offset, MAX_DISPLAY_LEN, "Vehicle = %d ", vehicle_count);
//
//        /* Now set the offsets where the string should appear */
//        txt_params->x_offset = 10;
//        txt_params->y_offset = 12;
//
//        /* Font , font-color and font-size */
//        txt_params->font_params.font_name = "Serif";
//        txt_params->font_params.font_size = 10;
//        txt_params->font_params.font_color.red = 1.0;
//        txt_params->font_params.font_color.green = 1.0;
//        txt_params->font_params.font_color.blue = 1.0;
//        txt_params->font_params.font_color.alpha = 1.0;
//
//        /* Text background color */
//        txt_params->set_bg_clr = 1;
//        txt_params->text_bg_clr.red = 0.0;
//        txt_params->text_bg_clr.green = 0.0;
//        txt_params->text_bg_clr.blue = 0.0;
//        txt_params->text_bg_clr.alpha = 1.0;
//
//        nvds_add_display_meta_to_frame(frame_meta, display_meta);
//        //显示帧数据、图像数据、文字等
//    }
//
//    g_print("Frame Number = %d Number of objects = %d "
//            "Vehicle Count = %d Person Count = %d\n",
//            frame_number, num_rects, vehicle_count, person_count);
//    frame_number++;
//    return GST_PAD_PROBE_OK;
//}

static gboolean//针对不同的消息类型进行相应的处理
bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = (GMainLoop *) data;
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("ERROR from element %s: %s\n",
                       GST_OBJECT_NAME (msg->src), error->message);
            if (debug)
                g_printerr("Error details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int
main(int argc, char *argv[]) {
    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL, *source = NULL, *h264parser = NULL,
            *decoder = NULL, *streammux = NULL, *sink = NULL, *pgie = NULL, *nvvidconv = NULL,
            *nvosd = NULL;
#ifdef PLATFORM_TEGRA
    GstElement *transform = NULL;
#endif
    GstBus *bus = NULL;
    guint bus_watch_id;
    GstPad *osd_sink_pad = NULL;

    /* Check input arguments */
    if (argc != 2) {
        g_printerr("Usage: %s <H264 filename>\n", argv[0]);
        return -1;
    }

    /* Standard GStreamer initialization */
    gst_init(&argc, &argv);             //首先我们调用了gstreamer的初始化函数
    loop = g_main_loop_new(NULL, FALSE);//创建一个循环体

    /* Create gstreamer elements */
    /* Create Pipeline element that will form a connection of other elements */
    pipeline = gst_pipeline_new("dstest1-pipeline");
    //Pipeline通过gst_pipeline_new创建，参数为pipeline的名字。

    /* Source element for reading from the file */
    source = gst_element_factory_make("filesrc", "file-source");
    //创建一个element  类型是filesrc，名字是file-source。
    //该element用于读取文件

    /* Since the data format in the input file is elementary h264 stream,
     * we need a h264parser */
    h264parser = gst_element_factory_make("h264parse", "h264-parser");
    //创建一个element  类型是h264parse，名字是h264-parser。
    //该element用于解析h264文件

    /* Use nvdec_h264 for hardware accelerated decode on GPU */
    decoder = gst_element_factory_make("nvv4l2decoder", "nvv4l2-decoder");
    //该element用于调用GPU硬件加速解码h264文件


    /* Create nvstreammux instance to form batches from one or more sources. */
    streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    //该element用于把输入按照参数处理成一系列的视频帧



    if (!pipeline || !streammux) {
        g_printerr("One element could not be created. Exiting.\n");
        return -1;
    }

    /* Use nvinfer to run inferencing on decoder's output,
     * behaviour of inferencing is set through config file */
    pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
    //对输入图像进行推理，通过推理的配置文件
    /* Use convertor to convert from NV12 to RGBA as required by nvosd */
    nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");
    //视频颜色格式转换
    /* Create OSD to draw on the converted RGBA buffer */
    nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    //处理RGBA buffer 绘制ROI等 识别对象的Bounding Box，边框
    //识别对象的文字标签（字体、颜色、标示框）

    /* Finally render the osd output */
#ifdef PLATFORM_TEGRA     //makefile里已经定义
    // 转换成 EGLImage instance 给nveglglessink使用
    transform = gst_element_factory_make("nvegltransform", "nvegl-transform");
    if (!transform) {
        g_printerr("One tegra element could not be created. Exiting.\n");
        return -1;
    }
#endif
    sink = gst_element_factory_make("nveglglessink", "nvvideo-renderer");

    if (!source || !h264parser || !decoder
        || !pgie || !nvvidconv || !nvosd || !sink
            ) {
        g_printerr("One element could not be created. Exiting.\n");
        return -1;
    }

    /* we set the input filename to the source element */
    g_object_set(G_OBJECT (source), "location", argv[1], NULL);
    //设置source的位置，source是视频文件
    g_object_set(G_OBJECT (streammux), "width", MUXER_OUTPUT_WIDTH, "height",
                 MUXER_OUTPUT_HEIGHT, "batch-size", 1,
                 "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
    //设置视频格式，如分辨率等
    /* Set all the necessary properties of the nvinfer element,
     * the necessary ones are : */
    g_object_set(G_OBJECT (pgie),
                 "config-file-path", "dstest1_pgie_config.txt", NULL);
    //设置配置文件的路径。该配置文件指示tensorRT转换后的文件等。
    /* we add a message handler */
    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));//
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);//指定消息处理函数
    gst_object_unref(bus);

    /* Set up the pipeline */
    /* we add all elements into the pipeline */
#ifdef PLATFORM_TEGRA
    gst_bin_add_many(GST_BIN (pipeline),
                     source, h264parser, decoder, streammux, pgie,
                     nvvidconv, nvosd, transform, sink, NULL);//element加入到pipeline
#else
    gst_bin_add_many(GST_BIN (pipeline),
                     source, h264parser, decoder, streammux, pgie,
                     nvvidconv, nvosd, sink, NULL);
#endif

#if 1
    GstPad *sinkpad, *srcpad;
    gchar pad_name_sink[16] = "sink_0";
    gchar pad_name_src[16] = "src";

    sinkpad = gst_element_get_request_pad(streammux, pad_name_sink);
    if (!sinkpad) {
        g_printerr("Streammux request sink pad failed. Exiting.\n");
        return -1;
    }
    //获取指定element中的指定pad  该element为 streammux
    srcpad = gst_element_get_static_pad(decoder, pad_name_src);
    if (!srcpad) {
        g_printerr("Decoder request src pad failed. Exiting.\n");
        return -1;
    }

    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link decoder to stream muxer. Exiting.\n");
        return -1;
    }
    //gst_pad_link
    gst_object_unref(sinkpad);
    gst_object_unref(srcpad);
#endif

    /* we link the elements together */
    /* file-source -> h264-parser -> nvh264-decoder ->
     * nvinfer -> nvvidconv -> nvosd -> video-renderer */

    if (!gst_element_link_many(source, h264parser, decoder, NULL)) {
        g_printerr("Elements could not be linked: 1. Exiting.\n");
        return -1;
    }

#ifdef PLATFORM_TEGRA
    if (!gst_element_link_many(streammux, pgie,
                               nvvidconv, nvosd, transform, sink, NULL)) {
        g_printerr("Elements could not be linked: 2. Exiting.\n");
        return -1;
    }
#else
    if (!gst_element_link_many(streammux, pgie,
                               nvvidconv, nvosd, sink, NULL)) {
        g_printerr("Elements could not be linked: 2. Exiting.\n");
        return -1;
    }
#endif

    /* Lets add probe to get informed of the meta data generated, we add probe to
     * the sink pad of the osd element, since by that time, the buffer would have
     * had got all the metadata. */
//    osd_sink_pad = gst_element_get_static_pad(nvosd, "sink");
//    if (!osd_sink_pad)
    g_print("Unable to get sink pad\n");
//    else
//        gst_pad_add_probe(osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
//                          osd_sink_pad_buffer_probe, NULL, NULL);
//osd_sink_pad_buffer_probe 创建探针 

//以上都是设置属性，连接Elements，设置消息等操作，先把整个的视频处理流程勾勒出来。

    /* Set the pipeline to "playing" state */
    g_print("Now playing: %s\n", argv[1]);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);//运行

    /* Wait till pipeline encounters an error or EOS */
    g_print("Running...\n");
    g_main_loop_run(loop);//bus_call 消息处理函数可以结束loop

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);//释放为pipeline分配的所有资源
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT (pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);//销毁loop对象
    return 0;
}

//失败项
#if 0
//
// Created by toson on 20-2-10.
//

#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include "gstnvdsmeta.h"

/* The muxer output resolution must be set if the input streams will be of
 * different resolution. The muxer will scale all the input frames to this
 * resolution. */
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080

/* Muxer batch formation timeout, for e.g. 40 millisec. Should ideally be set
 * based on the fastest source's framerate. */
#define MUXER_BATCH_TIMEOUT_USEC 4000000

static void cb_new_rtspsrc_pad(GstElement *element, GstPad *pad, gpointer data) {
    gchar *name;
    GstCaps *p_caps;
    gchar *description;
    GstElement *p_rtph264depay;

    name = gst_pad_get_name(pad);
    g_print("A new pad %s was created\n", name);

    // here, you would setup a new pad link for the newly created pad
    // sooo, now find that rtph264depay is needed and link them?
    p_caps = gst_pad_get_pad_template_caps(pad);

    description = gst_caps_to_string(p_caps);
    printf("%s\n", p_caps, ", ", description, "\n");
    g_free(description);

    p_rtph264depay = GST_ELEMENT(data);

    // try to link the pads then ...
    if (!gst_element_link_pads(element, name, p_rtph264depay, "sink")) {
        printf("Failed to link elements 3\n");
    }

    g_free(name);
}

/* osd_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
 * and update params for drawing rectangle, object information etc. */
//提取元数据从OSD slink绘制矩形框和物体信息等。
//static GstPadProbeReturn
//osd_sink_pad_buffer_probe(GstPad *pad, GstPadProbeInfo *info,
//                          gpointer u_data) {
//    GstBuffer *buf = (GstBuffer *) info->data;
//    guint num_rects = 0;
//    NvDsObjectMeta *obj_meta = NULL;
//    guint vehicle_count = 0;
//    guint person_count = 0;
//    NvDsMetaList *l_frame = NULL;
//    NvDsMetaList *l_obj = NULL;
//    NvDsDisplayMeta *display_meta = NULL;
//
//    NvDsBatchMeta *batch_meta = gst_buffer_get_nvds_batch_meta(buf);//包含多帧
//
//    for (l_frame = batch_meta->frame_meta_list; l_frame != NULL;
//         l_frame = l_frame->next) {
//        NvDsFrameMeta *frame_meta = (NvDsFrameMeta *) (l_frame->data);//帧数据
//        int offset = 0;
//        for (l_obj = frame_meta->obj_meta_list; l_obj != NULL;
//             l_obj = l_obj->next) {
//            obj_meta = (NvDsObjectMeta *) (l_obj->data);
//            if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE) {
//                vehicle_count++;
//                num_rects++;
//            }
//            if (obj_meta->class_id == PGIE_CLASS_ID_PERSON) {
//                person_count++;
//                num_rects++;
//            }
//        }//for循环统计该帧的车辆和行人
//        display_meta = nvds_acquire_display_meta_from_pool(batch_meta);
//        NvOSD_TextParams *txt_params = &display_meta->text_params[0];
//        display_meta->num_labels = 1;
//        txt_params->display_text = g_malloc0(MAX_DISPLAY_LEN);
//        offset = snprintf(txt_params->display_text, MAX_DISPLAY_LEN, "Person = %d ", person_count);
//        offset = snprintf(txt_params->display_text + offset, MAX_DISPLAY_LEN, "Vehicle = %d ", vehicle_count);
//
//        /* Now set the offsets where the string should appear */
//        txt_params->x_offset = 10;
//        txt_params->y_offset = 12;
//
//        /* Font , font-color and font-size */
//        txt_params->font_params.font_name = "Serif";
//        txt_params->font_params.font_size = 10;
//        txt_params->font_params.font_color.red = 1.0;
//        txt_params->font_params.font_color.green = 1.0;
//        txt_params->font_params.font_color.blue = 1.0;
//        txt_params->font_params.font_color.alpha = 1.0;
//
//        /* Text background color */
//        txt_params->set_bg_clr = 1;
//        txt_params->text_bg_clr.red = 0.0;
//        txt_params->text_bg_clr.green = 0.0;
//        txt_params->text_bg_clr.blue = 0.0;
//        txt_params->text_bg_clr.alpha = 1.0;
//
//        nvds_add_display_meta_to_frame(frame_meta, display_meta);
//        //显示帧数据、图像数据、文字等
//    }
//
//    g_print("Frame Number = %d Number of objects = %d "
//            "Vehicle Count = %d Person Count = %d\n",
//            frame_number, num_rects, vehicle_count, person_count);
//    frame_number++;
//    return GST_PAD_PROBE_OK;
//}

static gboolean//针对不同的消息类型进行相应的处理
bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = (GMainLoop *) data;
    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("ERROR from element %s: %s\n",
                       GST_OBJECT_NAME (msg->src), error->message);
            if (debug)
                g_printerr("Error details: %s\n", debug);
            g_free(debug);
            g_error_free(error);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int
main(int argc, char *argv[]) {
    GMainLoop *loop = NULL;
    GstElement *pipeline = NULL, *source = NULL, *rtppay = NULL, *h264parser = NULL,
            *decoder = NULL, *streammux = NULL, *sink = NULL, *pgie = NULL, *nvvidconv = NULL,
            *nvosd = NULL;
#ifdef PLATFORM_TEGRA
    GstElement *transform = NULL;
#endif
    GstBus *bus = NULL;
    guint bus_watch_id;
    GstPad *osd_sink_pad = NULL;

    /* Check input arguments */
    if (argc != 2) {
        g_printerr("Usage: %s <H264 filename>\n", argv[0]);
        return -1;
    }
    int is_rtspsrc = 0;
    if (argv[1][0] == 'r' && argv[1][1] == 't' && argv[1][2] == 's' && argv[1][3] == 'p' && argv[1][4] == ':') {
        g_printerr("Input is rtsp source.\n");
        is_rtspsrc = 1;
    }

    /* Standard GStreamer initialization */
    gst_init(&argc, &argv);             //首先我们调用了gstreamer的初始化函数
    loop = g_main_loop_new(NULL, FALSE);//创建一个循环体

    /* Create gstreamer elements */
    /* Create Pipeline element that will form a connection of other elements */
    pipeline = gst_pipeline_new("dstest1-pipeline");
    //Pipeline通过gst_pipeline_new创建，参数为pipeline的名字。

    /* Source element for reading from the file */
    source = gst_element_factory_make(is_rtspsrc ? "rtspsrc" : "filesrc", "file-source");
    //创建一个element  类型是filesrc，名字是file-source。
    //该element用于读取文件

    if (is_rtspsrc) {
        g_object_set(G_OBJECT (source), "latency", 2000, NULL);
        rtppay = gst_element_factory_make("rtph264depay", "depayl");
        if (!rtppay) {
            g_printerr("rtppay element could not be created.\n");
            return -1;
        }
    }

    /* Since the data format in the input file is elementary h264 stream,
     * we need a h264parser */
    h264parser = gst_element_factory_make("h264parse", "h264-parser");
    //创建一个element  类型是h264parse，名字是h264-parser。
    //该element用于解析h264文件

    /* Use nvdec_h264 for hardware accelerated decode on GPU */
    decoder = gst_element_factory_make("nvv4l2decoder", "nvv4l2-decoder");
    //该element用于调用GPU硬件加速解码h264文件


    /* Create nvstreammux instance to form batches from one or more sources. */
    streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    //该element用于把输入按照参数处理成一系列的视频帧



    if (!pipeline || !streammux) {
        g_printerr("One element could not be created 1. Exiting.\n");
        return -1;
    }

    /* Use nvinfer to run inferencing on decoder's output,
     * behaviour of inferencing is set through config file */
    pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
    //对输入图像进行推理，通过推理的配置文件
    /* Use convertor to convert from NV12 to RGBA as required by nvosd */
    nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");
    //视频颜色格式转换
    /* Create OSD to draw on the converted RGBA buffer */
    nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    //处理RGBA buffer 绘制ROI等 识别对象的Bounding Box，边框
    //识别对象的文字标签（字体、颜色、标示框）

    /* Finally render the osd output */
#ifdef PLATFORM_TEGRA     //makefile里已经定义
    transform = gst_element_factory_make("nvegltransform", "nvegl-transform");
    // 转换成 EGLImage instance 给nveglglessink使用
#endif
    sink = gst_element_factory_make("nveglglessink", "nvvideo-renderer");

    if (!source || !h264parser || !decoder
        || !pgie || !nvvidconv || !nvosd || !sink
            ) {
        g_printerr("One element could not be created 2. Exiting.\n");
        return -1;
    }

#ifdef PLATFORM_TEGRA
    if (!transform) {
        g_printerr("One tegra element could not be created. Exiting.\n");
        return -1;
    }
#endif

    /* we set the input filename to the source element */
    g_object_set(G_OBJECT (source), "location", argv[1], NULL);
    //设置source的位置，source是视频文件
    g_object_set(G_OBJECT (streammux), "width", MUXER_OUTPUT_WIDTH, "height",
                 MUXER_OUTPUT_HEIGHT, "batch-size", 1,
                 "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
    //设置视频格式，如分辨率等
    /* Set all the necessary properties of the nvinfer element,
     * the necessary ones are : */
    g_object_set(G_OBJECT (pgie),
                 "config-file-path", "dstest1_pgie_config.txt", NULL);
    //设置配置文件的路径。该配置文件指示tensorRT转换后的文件等。
    /* we add a message handler */
    bus = gst_pipeline_get_bus(GST_PIPELINE (pipeline));//
    bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);//指定消息处理函数
    gst_object_unref(bus);

    /* Set up the pipeline */
    /* we add all elements into the pipeline */
    gst_bin_add(GST_BIN (pipeline), source);
    if (is_rtspsrc) {
        gst_bin_add_many(GST_BIN (pipeline), rtppay, NULL);
        g_signal_connect(source, "pad-added", G_CALLBACK(cb_new_rtspsrc_pad), rtppay);
    }
    gst_bin_add_many(GST_BIN (pipeline), h264parser, decoder, streammux, pgie, NULL);
#ifdef PLATFORM_TEGRA
    gst_bin_add_many(GST_BIN (pipeline),
                     nvvidconv, nvosd, transform, sink, NULL);//element加入到pipeline
#else
    gst_bin_add_many(GST_BIN (pipeline),
                     nvvidconv, nvosd, sink, NULL);
#endif

#if 1
    GstPad *sinkpad, *srcpad;
    gchar pad_name_sink[16] = "sink_0";
    gchar pad_name_src[16] = "src";

    sinkpad = gst_element_get_request_pad(streammux, pad_name_sink);
    if (!sinkpad) {
        g_printerr("Streammux request sink pad failed. Exiting.\n");
        return -1;
    }
    //获取指定element中的指定pad  该element为 streammux
    srcpad = gst_element_get_static_pad(decoder, pad_name_src);
    if (!srcpad) {
        g_printerr("Decoder request src pad failed. Exiting.\n");
        return -1;
    }

    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link decoder to stream muxer. Exiting.\n");
        return -1;
    }
    //gst_pad_link
    gst_object_unref(sinkpad);
    gst_object_unref(srcpad);
#endif

    /* we link the elements together */
    /* file-source -> h264-parser -> nvh264-decoder ->
     * nvinfer -> nvvidconv -> nvosd -> video-renderer */
    if (is_rtspsrc) {
        if (!gst_element_link(rtppay, h264parser)) {
            printf("\nNOPE: gst_element_link(rtppay, h264parser)\n");
        }
        if (!gst_element_link_many(h264parser, decoder, NULL)) {
            g_printerr("Elements could not be linked: 1. Exiting.\n");
            return -1;
        }
    } else {
        if (!gst_element_link_many(source, h264parser, decoder, NULL)) {
            g_printerr("Elements could not be linked: 1. Exiting.\n");
            return -1;
        }
    }

#ifdef PLATFORM_TEGRA
    if (!gst_element_link_many(streammux, pgie,
                               nvvidconv, nvosd, transform, sink, NULL)) {
        g_printerr("Elements could not be linked: 2. Exiting.\n");
        return -1;
    }
#else
    if (!gst_element_link_many(streammux, pgie,
                               nvvidconv, nvosd, sink, NULL)) {
        g_printerr("Elements could not be linked: 2. Exiting.\n");
        return -1;
    }
#endif

    /* Lets add probe to get informed of the meta data generated, we add probe to
     * the sink pad of the osd element, since by that time, the buffer would have
     * had got all the metadata. */
//    osd_sink_pad = gst_element_get_static_pad(nvosd, "sink");
//    if (!osd_sink_pad)
    g_print("Unable to get sink pad\n");
//    else
//        gst_pad_add_probe(osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
//                          osd_sink_pad_buffer_probe, NULL, NULL);
//osd_sink_pad_buffer_probe 创建探针

//以上都是设置属性，连接Elements，设置消息等操作，先把整个的视频处理流程勾勒出来。

    /* Set the pipeline to "playing" state */
    g_print("Now playing: %s\n", argv[1]);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);//运行

    /* Wait till pipeline encounters an error or EOS */
    g_print("Running...\n");
    g_main_loop_run(loop);//bus_call 消息处理函数可以结束loop

    /* Out of the main loop, clean up nicely */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);//释放为pipeline分配的所有资源
    g_print("Deleting pipeline\n");
    gst_object_unref(GST_OBJECT (pipeline));
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);//销毁loop对象
    return 0;
}
#endif