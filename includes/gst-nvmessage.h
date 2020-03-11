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
 * <b>NVIDIA GStreamer DeepStream: Helper Queries</b>
 *
 * @b Description: This file specifies the NVIDIA DeepStream GStreamer helper
 * message functions.
 *
 */
#ifndef __GST_NVMESSAGE_H__
#define __GST_NVMESSAGE_H__

#include <gst/gst.h>

G_BEGIN_DECLS


/**
 * @defgroup gst_message_plugin Message Functions
 * Helper functions for custom GStreamer messages posted by DeepStream GStreamer
 * plugins.
 *
 * DeepStream GStreamer plugins post the following custom messages:
 * - Stream EOS - Posted by the `NvStreamMuxer` element when it recieves EOS
 *                on one of its sink pads.
 *
 * @ingroup gstreamer_utilities_group
 * @{
 */

/**
 * Creates a new Stream EOS message.
 *
 * params[in] obj The `GStreamer` object creating the message.
 * params[in] eos_stream_id ID of stream for which EOS has been received.
 *
 * @return A pointer to the newly created message.
 */
GstMessage * gst_nvmessage_new_stream_eos (GstObject *obj, guint eos_stream_id);

/**
 * Determines whether a message is a Stream EOS message.
 *
 * params[in] message The `GstMessage` to be checked.
 *
 * @return A boolean indicating whether the message is a stream eos message.
 */
gboolean gst_nvmessage_is_stream_eos (GstMessage * message);

/**
 * Parses the stream ID from a Stream EOS message. This ID is the index of the
 * stream which sent the EOS event to the stream muxer.
 *
 * params[in] message A Stream EOS message.
 * params[out] eos_stream_id A pointer to an unsigned integer in which the function
 *                        is to place the stream ID.
 *
 * return A boolean indicating whether the message was successfully parsed.
 */
gboolean gst_nvmessage_parse_stream_eos (GstMessage * message, guint * eos_stream_id);

/** @} */

G_END_DECLS

#endif
