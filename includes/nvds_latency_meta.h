/**
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

#ifndef _NVDSMETA_LATENCY_H_
#define _NVDSMETA_LATENCY_H_

#include "glib.h"
#include "gmodule.h"
#include "nvdsmeta.h"

#define MAX_COMPONENT_LEN 64

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Holds information about latency of the given component
 */
typedef struct _NvDsMetaCompLatency {
  /**
   * name of the component for which latency is measured
   */
  gchar component_name[MAX_COMPONENT_LEN];
  /**
   * system timestamp of buffer when it arrives at the input of the component
   */
  gdouble in_system_timestamp;
  /**
   * system timestamp of buffer when it is sent out to the downstream component
   */
  gdouble out_system_timestamp;
  /**
   * source id of the component e.g. camera_id
   */
  guint source_id;
  /**
   * current frame number for which latency is measured
   */
  guint frame_num;
  /**
   *  pad or port index of stream muxer for the frame in the batch
   */
  guint pad_index;
}NvDsMetaCompLatency;

/**
 * Holds information about latency of the given frame
 */
typedef struct
{
  /**
   * source id of the component e.g. camera_id
   */
  guint source_id;
  /**
   * current frame number for which latency is measured
   */
  guint frame_num;
  /**
   * system timestamp of buffer when it arrives at the input of the first
   * component in the pipeline. By default, decoder is considered to be the
   * first component in the pipeline.
   */
  gdouble comp_in_timestamp;
  /**
   * the latency of the frame in ms
   */
  gdouble latency;
} NvDsFrameLatencyInfo;

/**
 * Sets the system timestamp when the gst buffer arrives at the input of the
 * component.
 *
 * @param[in] buffer GstBuffer which arrives at the input of the component
 * @param[in] element_name pointer to the name of the component for which
 *            latency is to be measured
 *
 * @return A pointer to @ref NvDsUserMeta structure.
 * It holds NvDsMetaCompLatency as @a user_meta_data
 */

NvDsUserMeta *nvds_set_input_system_timestamp(GstBuffer * buffer,
    gchar *element_name);

/**
 * Sets the system timestamp when the gst buffer is pushed to the downstream
 * component. It is a corresponding call to nvds_set_input_system_timestamp()
 *
 * @param[in] buffer GstBuffer which is pushed to the downstream component
 * @param[in] element_name A pointer to the name of the component for which
 *            latency is to be measured
 *
 * returns TRUE if output timestamp is attached successfully, otherwise FALSE
 */
gboolean nvds_set_output_system_timestamp(GstBuffer * buffer, gchar *element_name);

/**
 * Measures the latency of all the frames present in the current batch
 * The latency will be computed till the point @a buf reaches.
 * User can install the probe on the either pads of the component and call
 * this API to measure the latency.
 *
 * @param[in]  buf GstBuffer which has @ref NvDsBatchMeta attched as metadata
 * @param[out] latency_info A pointer to NvDsFrameLatencyInfo.
 *             It should be allocated as per batch-size. It will contain latency
 *             information of all the sources when the function returns.
 */
guint nvds_measure_buffer_latency(GstBuffer *buf,
    NvDsFrameLatencyInfo *latency_info);

/**
 * @return a boolean set to TRUE if environment variable
 * NVDS_ENABLE_LATENCY_MEASUREMENT is exported
 */
gboolean nvds_get_enable_latency_measurement(void);
/**
 * @return a boolean set to TRUE if environment variable
 * NVDS_ENABLE_LATENCY_MEASUREMENT is exported
 */

/**
 * variable holding the return value of @ref nvds_get_enable_latency_measurement()
 */
#define nvds_enable_latency_measurement (nvds_get_enable_latency_measurement())
/** @} */
#ifdef __cplusplus
}
#endif
#endif
