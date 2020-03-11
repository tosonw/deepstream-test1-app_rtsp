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
#ifndef _NVDS_OPTICALFLOW_META_H_
#define _NVDS_OPTICALFLOW_META_H_

#include <gst/gst.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Holds information about motion vector information of an element
 */
typedef struct _NvOFFlowVector
{
    /** x component of motion vector */
    gshort flowx;

    /** y component of motion vector */
    gshort flowy;
} NvOFFlowVector;

/**
 * Holds information optical flow metadata information of a frame
 */
typedef struct
{
    /** number of rows present in the frame for given block size
     *  e.g. if block size is 4 and frame height is 720, then
     *  number of rows = (720 / 4) = 180
     */
    guint rows;
    /** number of columns present in the frame for given block size
     *  e.g. if block size is 4 and frame width is 1280, then
     *  number of columns = (1280 / 4) = 320
     */
    guint cols;
    /** size of motion vector. Refer @ref NvOFFlowVector */
    guint mv_size;
    /** current frame number of the source */
    gulong frame_num;
    /** motion vector data pointer */
    void *data;
    /** reserved field, for internal purpose only */
    void *priv;
    /** reserved field, for internal purpose only */
    void *reserved;
} NvDsOpticalFlowMeta;

#ifdef __cplusplus
}
#endif

#endif
