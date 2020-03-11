/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

/**
 * @file
 * <b>NVIDIA GStreamer DeepStream: Metadata Extension</b>
 *
 * @b Description: This file specifies the Metadata structure used to
 * carry DeepStream Metadata or any other metadata in gstreamer pipeline
 */

/**
 * @defgroup gstreamer_metagroup_api DeepStream Metadata Extension
 *
 * Defines an API for managing GStreamer DeepStream metadata.
 * @ingroup gstreamer_metadata_group
 * @{
 *
 * DeepStream Metadata is attached to a buffer with gst_buffer_add_nvds_meta().
 * It's metadata type is set to NVDS_BATCH_GST_META
 *
 * Multiple metadatas may be attached by different elements.
 * gst_buffer_get_nvds_meta() gets the last added @ref NvDsMeta.
 * To iterate through each NvDsMeta, following snippet can be used.
 *
 * @code
 *  gpointer state = NULL;
 *  GstMeta *gst_meta;
 *  NvDsMeta *nvdsmeta;
 *  static GQuark _nvdsmeta_quark = 0;
 *
 *  if (!_nvdsmeta_quark)
 *    _nvdsmeta_quark = g_quark_from_static_string(NVDS_META_STRING);
 *
 *  while ((gst_meta = gst_buffer_iterate_meta (buf, &state))) {
 *    if (gst_meta_api_type_has_tag(gst_meta->info->api, _nvdsmeta_quark)) {
 *      nvdsmeta = (NvDsMeta *) gst_meta;
 *      // Do something with this nvdsmeta
 *    }
 *  }
 * @endcode
 *
 *  The meta_data member of the NvDsMeta structure must be cast to a meaningful structure pointer
 *  based on the meta_type. For example, for `meta_type = NVDS_BATCH_GST_META`,
 *  meta_data must be cast as `(NvDsBatchMeta *)`.
 */

#ifndef GST_NVDS_META_API_H
#define GST_NVDS_META_API_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/base/gstbasetransform.h>

#include "nvdsmeta.h"
#include "nvds_latency_meta.h"

#ifdef __cplusplus
extern "C"
{
#endif
GType nvds_meta_api_get_type (void);
#define NVDS_META_API_TYPE (nvds_meta_api_get_type())

const GstMetaInfo *nvds_meta_get_info (void);

#define NVDS_META_INFO (nvds_meta_get_info())

#define NVDS_META_STRING "nvdsmeta"

/**
 * Specifies the type of meta data. NVIDIA defined GstNvDsMetaType will be present
 * in the range from @ref NVDS_BATCH_GST_META to @ref NVDS_START_USER_META
 * (refer nvdsmeta.h)
 * User can add it's own metadata type NVDS_RESERVED_GST_META onwards.
 */
typedef enum {
  NVDS_GST_INVALID_META=-1,
  /* contains information of formed batch */
  NVDS_BATCH_GST_META = NVDS_GST_CUSTOM_META + 1,
  NVDS_DECODER_GST_META,
  /* contains information of dewarped surfaces */
  NVDS_DEWARPER_GST_META,
  NVDS_RESERVED_GST_META = NVDS_GST_CUSTOM_META + 4096,
  /* User can start adding it's own gst metadata from here onwards */
  NVDS_GST_META_FORCE32 = 0x7FFFFFFF
} GstNvDsMetaType;
/**
 * Specifies the type of function to copy meta data.
 * It is passed the pointer to meta data and user specific data.
 * It allocates the required memory, copy the content and returns
 * the pointer to newly allocated memory.
 */

/**
 * Holds DeepSteam meta data.
 * */
 typedef struct _NvDsMeta {
  GstMeta       meta;

  /** Must be cast to another structure based on @a meta_type. */
  gpointer meta_data;

  gpointer user_data;

  /** Type of metadata, from the @ref meta_type enum. */
  gint meta_type;

  /**
   * It is called when meta_data needs to copied / transformed
   * from one buffer to other. meta_data and user_data are passed as arguments.
   */

  NvDsMetaCopyFunc copyfunc;
  /**
   * It is called when meta_data is going to be destroyed.
   * meta_data and user_data are passed as arguments.
   */
  NvDsMetaReleaseFunc freefunc;

  /**
   * It is called when @a meta_data is transformed into @a nvds user meta.
   * This function must be provided by the gstreamer plugin which is present
   * before @a nvstreammux in the deepstream pipeline.
   * @a nvstreammux component copies the @a meta_data inside @ref NvDsMeta to
   * user meta data at frame level inside @ref NvDsFrameMeta.
   * meta_data and user_data are passed as arguments.
   *
   * To retrive the content of @a meta_data, user must iterate over
   * NvDsFrameMetaList.Then, search for @a meta_type of @ref NvDsUserMeta
   * which the user has attached. (Refer deepstream-gst-metadata-test source
   * code for more details)
   *
   * meta_data and user_data are passed as arguments.
   */

  NvDsMetaCopyFunc gst_to_nvds_meta_transform_func;
  /**
   * It is called when nvdsmeta, transformed from gst meta, attached by plugin
   * before nvstreammux going to be destroyed.
   * meta_data and user_data are passed as arguments.
   */
  NvDsMetaReleaseFunc gst_to_nvds_meta_release_func;

} NvDsMeta;

/**
 * Adds GstMeta of type @ref NvDsMeta to the GstBuffer and sets the `meta_data`
 * member of @ref NvDsMeta.
 *
 * @param[in] buffer GstBuffer to which the function adds metadata.
 * @param[in] meta_data The pointer to which the function sets the meta_data
 *            member of @ref NvDsMeta.
 * @param[in] user_data A pointer to the user specific data
 * @param[in] copy_func The NvDsMetaCopyFunc function to be called when
 *            NvDsMeta is to be copied. The function is called with
 *            meta_data and user_data as parameters.
 * @param[in] release_func The NvDsMetaReleaseFunc function to be called when
 *            NvDsMeta is to be destroyed. The function is called with
 *            meta_data and user_data as parameters.
 *
 * @return A pointer to the attached @ref NvDsMeta structure; or NULL in case
 * of failure.
 */

NvDsMeta *gst_buffer_add_nvds_meta (GstBuffer *buffer, gpointer meta_data,
    gpointer user_data, NvDsMetaCopyFunc copy_func,
    NvDsMetaReleaseFunc release_func);

/** Gets the @ref NvDsMeta last added to the GstBuffer.
 *
 * @param[in] buffer GstBuffer
 *
 * @return A pointer to the last added NvDsMeta structure; or NULL if no
 * NvDsMeta was attached.
 */
NvDsMeta* gst_buffer_get_nvds_meta (GstBuffer *buffer);

/** Gets the NvDsBatchMeta added to the GstBuffer.
 *
 * @param[in] buffer GstBuffer
 *
 * @return A pointer to the NvDsBatchMeta structure; or NULL if no
 * NvDsMeta was attached.
 */
NvDsBatchMeta * gst_buffer_get_nvds_batch_meta (GstBuffer *buffer);
/** @} */
#ifdef __cplusplus
}
#endif
#endif
