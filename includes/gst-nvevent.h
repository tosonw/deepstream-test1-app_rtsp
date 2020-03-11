/*
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/**
 * @file
 * <b>NVIDIA GStreamer DeepStream: Custom Events</b>
 *
 * @b Description: This file specifies the NVIDIA DeepStream GStreamer custom
 * event functions, useful to maps events to individual sources which
 * are batched together by nvstreammux.
 *
 */
#ifndef __GST_NVEVENT_H__
#define __GST_NVEVENT_H__

#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FLAG(name) GST_EVENT_TYPE_##name
/** Defines the types of custom events supported. */
typedef enum {
  GST_NVEVENT_PAD_ADDED
    = GST_EVENT_MAKE_TYPE (400, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),/**< Custom event to indicate Pad Added*/
  GST_NVEVENT_PAD_DELETED
    = GST_EVENT_MAKE_TYPE (401, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),/**< Custom event to indicate Pad Deleted */
  GST_NVEVENT_STREAM_EOS
    = GST_EVENT_MAKE_TYPE (402, FLAG(DOWNSTREAM) | FLAG(SERIALIZED)),/**< Custom event to indicate EOS of particular stream in a batch */
  GST_NVEVENT_STREAM_SEGMENT
    = GST_EVENT_MAKE_TYPE (403, FLAG(DOWNSTREAM) | FLAG(SERIALIZED))/**< Custom event to indicate stream segment*/
} GstNvEventType;
#undef FLAG

/**
 * Creates a custom pad added for particular source
 *
 * source_id[in] source_id of the stream added, which is also the pad_id of the
 *               sinkpad of nvstreammux for which the source is configured
 *
 * @return GstEvent corresponding to request else NULL
 */
GstEvent * gst_nvevent_new_pad_added (guint source_id);

/**
 * Creates a custom pad deleted event for particular source
 *
 * source_id[in] source_id of the stream to be removed from pipeline, which also
 *               the pad_id of the sinkpad of the nvstreammux for which the
 *               source is configured
 *
 * @return GstEvent corresponding to request or else NULL
 */
GstEvent * gst_nvevent_new_pad_deleted (guint source_id);

/**
 * Create a custom EOS event for a particular source_id
 *
 * source_id[in] source_id for which EOS is to be sent, which is also the pad_id
 *               of the sinkpad of the nvstreammux for which the source is
 *               configured.
 *
 * @return GstEvent corresponding to request or else NULL
 */
GstEvent * gst_nvevent_new_stream_eos (guint source_id);

/**
 * Create a custom segment event for a particular source
 *
 * source_id[in] source_id for which a segment event is to be sent, which is
 *               also the pad_id of the sinkpad of the nvstreammux for which
 *               the source is configured.
 * segment[in]   Copy of the segment which needs to be sent along with the custom
 *               event, which corresponds to original segment for the particular
 *               source
 *
 * @return GstEvent corresponding to request or else NULL
 */
GstEvent * gst_nvevent_new_stream_segment (guint source_id, GstSegment *segment);

/**
 * Parse the pad added event received on sinkpad
 *
 * event[in]      The event received on the sinkpad when a particular pad is
 *                added to nvstreammux
 *
 * source_id[out] parsed source_id for which the event is sent
 *
 *
 * @return void.
 */
void gst_nvevent_parse_pad_added (GstEvent * event, guint * source_id);


/**
 * Parse the pad deleted event received on sinkpad
 *
 * event[in]      The event received on the sinkpad when a particular pad is
 *                deleted from nvstreammux
 *
 * source_id[out] parsed source_id for which the event is sent
 *
 *
 * @return void.
 */
void gst_nvevent_parse_pad_deleted (GstEvent * event, guint * source_id);

/**
 * Parse the stream EOS event received on sinkpad
 *
 * event[in]      The event received on the sinkpad when a particular source_id
 *                sends a EOS event
 *
 * source_id[out] parsed source_id for which the event is sent
 *
 *
 * @return void.
 */
void gst_nvevent_parse_stream_eos (GstEvent * event, guint * source_id);

/**
 * Parse the stream segment event received on sinkpad
 *
 * event[in]      The event received on the sinkpad when a particular source_id
 *                sends a segment event
 *
 * source_id[out] parsed source_id for which the event is sent
 *
 * segment [out]  parsed segment corresponding to source _id for which the event
 *                is sent
 *
 * @return void.
 */
void gst_nvevent_parse_stream_segment (GstEvent * event, guint * source_id,
    GstSegment **segment);


#ifdef __cplusplus
}
#endif

#endif
