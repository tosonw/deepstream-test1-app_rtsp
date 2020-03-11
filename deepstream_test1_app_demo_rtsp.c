//
// Created by toson on 20-2-10.
//

#include "stdio.h"
#include "gst/gst.h"

#define RTSPCAM "rtsp://admin:admin123@192.168.1.106:554/cam/realmonitor?channel=1&subtype=0"
#define MUXER_OUTPUT_WIDTH 1920
#define MUXER_OUTPUT_HEIGHT 1080
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
    printf("%s\n", p_caps);
    printf("%s\n", description);
    g_free(description);

    p_rtph264depay = GST_ELEMENT(data);

    // try to link the pads then ...
    if (!gst_element_link_pads(element, name, p_rtph264depay, "sink")) {
        printf("Failed to link elements 3\n");
    }

    g_free(name);
}


int main(int argc, char *argv[]) {
    GstElement *pipeline = NULL, *source = NULL, *rtppay = NULL, *parse = NULL,
            *decoder = NULL, *sink = NULL, *filter1 = NULL;
    GstCaps *filtercaps = NULL;

    gst_init(&argc, &argv);

    /// Build Pipeline
    pipeline = gst_pipeline_new("Toson");

    /// Create elements
    source = gst_element_factory_make("rtspsrc", "source");
    g_object_set(G_OBJECT (source), "latency", 2000, NULL);
    rtppay = gst_element_factory_make("rtph264depay", "depayl");
    parse = gst_element_factory_make("h264parse", "parse");
#ifdef PLATFORM_TEGRA
    decoder = gst_element_factory_make("nvv4l2decoder", "nvv4l2-decoder");
    GstElement *streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    GstElement *pgie = gst_element_factory_make("nvinfer", "primary-nvinference-engine");
    GstElement *nvvidconv = gst_element_factory_make("nvvideoconvert", "nvvideo-converter");
    GstElement *nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    GstElement *transform = gst_element_factory_make("nvegltransform", "nvegl-transform");
    sink = gst_element_factory_make ( "nveglglessink", "sink");
    if (!pipeline || !streammux || !pgie || !nvvidconv || !nvosd || !transform) {
        g_printerr("One element could not be created. Exiting.\n");
        return -1;
    }
    g_object_set(G_OBJECT (streammux), "width", MUXER_OUTPUT_WIDTH, "height",
                 MUXER_OUTPUT_HEIGHT, "batch-size", 1,
                 "batched-push-timeout", MUXER_BATCH_TIMEOUT_USEC, NULL);
    g_object_set(G_OBJECT (pgie),
                 "config-file-path", "dstest1_pgie_config.txt", NULL);
#else
    decoder = gst_element_factory_make("avdec_h264", "decode");
    sink = gst_element_factory_make("autovideosink", "sink");
#endif
    if (!pipeline || !source || !rtppay || !parse || !decoder || !sink) {
        g_printerr("One element could not be created.\n");
    }
    g_object_set(G_OBJECT (sink), "sync", FALSE, NULL);
    g_object_set(GST_OBJECT(source), "location", RTSPCAM, NULL);

    /// 加入插件
#ifdef PLATFORM_TEGRA
    gst_bin_add_many(GST_BIN (pipeline),
                     source, rtppay, parse, decoder, streammux, pgie,
                     nvvidconv, nvosd, transform, sink, NULL);
#else
    gst_bin_add_many(GST_BIN (pipeline),
                     source, rtppay, parse, decoder, sink, NULL);
#endif
    // listen for newly created pads
    g_signal_connect(source, "pad-added", G_CALLBACK(cb_new_rtspsrc_pad), rtppay);

#ifdef PLATFORM_TEGRA
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

    /// 链接插件
#ifdef PLATFORM_TEGRA
    if (!gst_element_link_many(rtppay, parse, decoder, NULL)) {
        printf("\nFailed to link elements 0.\n");
        return -1;
    }
    if (!gst_element_link_many(streammux, pgie, nvvidconv, nvosd, transform, sink, NULL)) {
        printf("\nFailed to link elements 2.\n");
        return -1;
    }
#else
    if (!gst_element_link_many(rtppay, parse, decoder, sink, NULL)) {
        printf("\nFailed to link elements.\n");
        return -1;
    }
#endif

    /// 开始运行
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    GstBus *bus = gst_element_get_bus(pipeline);
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                 (GstMessageType) (GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    if (msg != NULL) {
        gst_message_unref(msg);
    }
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

}
