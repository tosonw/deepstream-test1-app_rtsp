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
#ifndef _NVDS_DEWARPER_META_H_
#define _NVDS_DEWARPER_META_H_

#include <gst/gst.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Defines DeepStream Dewarper metadata.
 */

/**
 * Maximum number of dewarped surfaces per frame supported
 */
#define MAX_DEWARPED_VIEWS 4

typedef enum
{
    NVDS_META_SURFACE_NONE=0,
    /**
     * pushbroom surface surface type
     */
    NVDS_META_SURFACE_FISH_PUSHBROOM=1,
    /**
     * vertical radical cylindrical surface
     */
    NVDS_META_SURFACE_FISH_VERTCYL=2,
} NvDsSurfaceType;

typedef struct _NvDewarperSurfaceMeta {
  /**
   * an array of type of dewarped surfaces
   */
  guint type[MAX_DEWARPED_VIEWS];
  /**
   * an array of index of dewarped surfaces
   */
  guint index[MAX_DEWARPED_VIEWS];
  /**
   * source id of the frame e.g. camera id
   */
  guint source_id;
  /**
   * number of filled surfaces in the frame
   */
  guint num_filled_surfaces;
}NvDewarperSurfaceMeta;

#ifdef __cplusplus
}
#endif

#endif
