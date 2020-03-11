/*
 * Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/**
 * @file nvdsinfer_dbscan.h
 * <b>NVIDIA DeepStream DBScan based Object Clustering API </b>
 *
 * @b Description: This file specifies the APIs to use DBScan based object
 * clustering algorithm.
 *
 */
#ifndef __NVDSINFER_DBSCAN_H__
#define __NVDSINFER_DBSCAN_H__

#include <stddef.h>
#include <stdint.h>

#include <nvdsinfer.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque structure for the DBScan object clustering context.
 */
struct NvDsInferDBScan;
/**
 * DBScan clustering context handle.
 */
typedef struct NvDsInferDBScan *NvDsInferDBScanHandle;

/**
 * Holds the object clustering parameters required by DBSCAN.
 */
typedef struct
{
    float eps;
    uint32_t minBoxes;
    /** Boolean indicating whether Area-To-Hit-Ratio filter is enabled.
     * ATHR is calculated as: ATHR = sqrt(clusterArea) / nObjectsInCluster. */
    int enableATHRFilter;
    /** Area to hit ratio threshold. */
    float thresholdATHR;
} NvDsInferDBScanClusteringParams;

/**
 * Create a new DBScan object clustering context.
 *
 * @return Handle to the newly created context.
 */
NvDsInferDBScanHandle NvDsInferDBScanCreate();

/**
 * Destroy a DBScan object clustering context.
 *
 * @param[in] handle Handle to the context to be destroyed.
 */
void NvDsInferDBScanDestroy(NvDsInferDBScanHandle handle);

/**
 * Cluster an array of objects using given clustering parameters in-place.
 *
 * @param[in] handle Handle to the context be used for clustering.
 * @param[in] params Clustering paramaters.
 * @param[in,out] objects Array of objects to be clustered. The function will
 *                        place the clustered objects in the same array.
 * @param[in,out] numObjects Number of valid objects in the objects array. The
 *                           function will set the value after clustering.
 */
void NvDsInferDBScanCluster(NvDsInferDBScanHandle handle,
        NvDsInferDBScanClusteringParams *params,  NvDsInferObjectDetectionInfo *objects,
        size_t *numObjects);

#ifdef __cplusplus
}
#endif

#endif
