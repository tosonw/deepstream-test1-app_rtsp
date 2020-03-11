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

#ifndef _NVMOTRACKER_H_
#define _NVMOTRACKER_H_

#include <stdint.h>
#include <time.h>

#include "nvbufsurface.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup NvMOT API
 *  @{
 */

#define NVMOT_MAX_TRANSFORMS 4

typedef uint64_t NvMOTStreamId;

/**
 * @brief Compute target flags
 *
 * Multiple targets can be selected, and the tracker will optimize across them.
 * Combinations are allowed, e.g. NVTCOMP_GPU or NVTCOMP_PVA
 */
#define NVMOTCOMP_GPU     0x01
#define NVMOTCOMP_CPU     0x02
#define NVMOTCOMP_PVA     0x04
#define NVMOTCOMP_ANY     0xff
#define NVMOTCOMP_DEFAULT NVMOTCOMP_ANY
typedef uint8_t NvMOTCompute;

/**
 * @brief Configuration for batches for each input transform (scaling/color conversion)
 *
 */
typedef struct _NvMOTPerTransformBatchConfig
{
    NvBufSurfaceMemType bufferType;    /**< Type of buffer. */
    uint32_t maxWidth;                 /**< Maximum width of each frame. */
    uint32_t maxHeight;                /**< Maximum height of each frame. */
    uint32_t maxPitch;                 /**< Maximum pitch of each buffer. */
    uint32_t maxSize;                  /**< Maximum size of the buffer in bytes. */
    uint32_t colorFormat;              /**< Color format: RGB, NV12 etc. */
} NvMOTPerTransformBatchConfig;


/**
 * @brief Miscellaneous configurations
 *
 */
typedef struct _NvMOTMiscConfig
{
    uint32_t gpuId;            /**< GPU to be used. */
    uint32_t maxObjPerStream;  /**< Max number of objects to track per stream. 0 means no limit. */
    uint32_t maxObjPerBatch;   /**< Max number of objects to track per batch. 0 means no limit. */
    typedef void (*logMsg) (int logLevel, const char * format, ...);  /**< Callback for logging messages. */
} NvMOTMiscConfig;

/**
 * @brief Tracker configuration
 *
 * Configuration options for the tracker, applied to the whole context.
 *
 * @note This config is supposed to be deep-copied when passed to be used persistently
 */
typedef struct _NvMOTConfig
{
    NvMOTCompute computeConfig;      	/**< Compute target. see NvMOTCompute */
    uint32_t maxStreams;               	/**< Maximum number of streams in a batch. */
    uint8_t numTransforms;           	/**< Number of NvMOTPerTransformBatchConfig entries in perTransformBatchConfig */
    NvMOTPerTransformBatchConfig *perTransformBatchConfig; /**< List of numTransform batch configs including type and resolution,
                                                                one for each transform*/
    NvMOTMiscConfig miscConfig;       	/**< Miscellaneous configs */
    uint16_t customConfigFilePathSize; 	/**< The char length in customConfigFilePath */
    char* customConfigFilePath;      	/**< Path to the tracker's custom config file. Null terminated */
} NvMOTConfig;

/**
 * @brief Configuration request return status
 */
typedef enum
{
    NvMOTConfigStatus_OK,
    NvMOTConfigStatus_Error,
    NvMOTConfigStatus_Invalid,
    NvMOTConfigStatus_Unsupported
} NvMOTConfigStatus;

/**
 * @brief Tracker configuration status
 *
 * Status of configuration request, both summary and per-config status.
 */
typedef struct _NvMOTConfigResponse
{
    NvMOTConfigStatus summaryStatus; /**< Summary status of the entire configuration request. */
    NvMOTConfigStatus computeStatus; /**< Compute target request status. */
    NvMOTConfigStatus transformBatchStatus; /**< Transform batch config request status. Summary status for all transforms. */
    NvMOTConfigStatus miscConfigStatus;    /**< Status of the miscellaneous configs. */
    NvMOTConfigStatus customConfigStatus;  /**< Status of the custom configs. */
} NvMOTConfigResponse;

/**
 * @brief Generic status for tracking operations
 */
typedef enum
{
    NvMOTStatus_OK,
    NvMOTStatus_Error,
    NvMOTStatus_Invalid_Path
} NvMOTStatus;

/**
 * @brief Rectangle
 */
typedef struct _NvMOTRect
{
    int x;           /**< Pixel coordinate of left edge of the object bounding box. */
    int y;           /**< Pixel coordinate of top edge of the object bounding box. */
    int width;       /**< Width of the bounding box in pixels. */
    int height;      /**< Height of the bounding box in pixels. */
} NvMOTRect;

/**
 * @brief Information for each object to be tracked
 */
typedef struct _NvMOTObjToTrack
{
    uint16_t classId;     /**< Class of the object to be tracked. */
    NvMOTRect bbox;       /**< Bounding box. */
    float confidence;     /**< Detection confidence of the object. */
    bool doTracking;      /**< True: track this object.  False: do not initiate  tracking on this object. */
    void *pPreservedData; /**< Used for the client to keep track of any data associated with the object. */
} NvMOTObjToTrack;

/**
 * @brief List of objects
 */
typedef struct _NvMOTObjToTrackList
{
    bool detectionDone;     /**< True if detection was done on this frame even if the list of
                                 objects to track is empty. False otherwise. */
    NvMOTObjToTrack* list;  /**< Pointer to a list/array of object info blocks. */
    uint32_t numAllocated;  /**< Number of blocks allocated for the list. */
    uint32_t numFilled;     /**< Number of populated blocks in the list. */
} NvMOTObjToTrackList;

/**
 * @brief Frame containing the image and objects to be tracked
 *
 * @note numBuffers in NvMOTFrame is supposed to be smaller or equal to numTransforms in NvMOTConfig
 * @note the metadata in NvBufSurfaceParams of each bufferList in NvMOTFrame should be checked
 * 		  with the params specified in perTransformBatchConfig in NvMOTConfig
 */
typedef struct _NvMOTFrame
{
    NvMOTStreamId streamID;    /**< The stream source for this frame. */
    uint32_t frameNum;    /**< Frame number sequentially identifying the frame within a stream. */
    time_t timeStamp;     /**< Timestamp of the frame at the time of capture. */
    bool timeStampValid;  /**< The timestamp value is properly populated. */
    bool doTracking;      /**< True: track objects in this frame; False: do not track this frame. */
    bool reset;           /**< True: reset tracking for the stream. */
    uint8_t numBuffers;           /**< Number of entries in bufferList. */
    NvBufSurfaceParams** bufferList; /**< Array of pointers to buffer params. */
    NvMOTObjToTrackList objectsIn;  /**< List of objects to be tracked in this frame. BBoxes are scaled for the first buffer config */
} NvMOTFrame;

/**
 * @brief Information for each tracked object
 */
typedef struct _NvMOTTrackedObj
{
    uint16_t classId;     /**< Class of the object to be tracked. */
    uint64_t trackingId;  /**< Unique ID for the object as assigned by the tracker. */
    NvMOTRect bbox;       /**< Bounding box. */
    float confidence;     /**< Tracking confidence of the object. */
    uint32_t age;         /**< Track length in frames. */
    NvMOTObjToTrack *associatedObjectIn; /**< The associated input object if there is one. */
    uint8_t reserved[128]; 
} NvMOTTrackedObj;

/**
 * @brief List of tracked objects
 */
typedef struct _NvMOTTrackedObjList 
{
    NvMOTStreamId streamID;      /**< Stream associated with objects in the list. */
    uint32_t frameNum;    /**< Frame number for objects in the list. */
    bool valid;             /**< This entry in the batch is valid */
    NvMOTTrackedObj* list;  /**< Pointer to a list/array of object info blocks */
    uint32_t numAllocated;  /**< Number of blocks allocated for the list. */
    uint32_t numFilled;     /**< Number of populated blocks in the list. */
} NvMOTTrackedObjList;

/**
 * @brief Batch of lists of tracked objects
 */
typedef struct _NvMOTTrackedObjBatch
{
    NvMOTTrackedObjList *list;  /**< Pointer to array of obj lists. */
    uint32_t numAllocated;  /**< Number of blocks allocated for the list. */
    uint32_t numFilled;     /**< Number of filled blocks in the list. */
} NvMOTTrackedObjBatch;

/**
 * @brief Parameters for processing each batch
 *
 * See NvMOTProcessFrame
 */
typedef struct _NvMOTProcessParams
{
    uint32_t numFrames;            /**< Number of frames in the batch. */
    NvMOTFrame *frameList;     /**< Pointer to an array of frame data. */
} NvMOTProcessParams;

typedef struct _NvMOTQuery
{
    NvMOTCompute computeConfig;      	/**< Supported compute targets. see NvMOTCompute */
    uint8_t numTransforms;           	/**< Number of NvMOTPerTransformBatchConfig entries in perTransformBatchConfig */
    NvBufSurfaceColorFormat colorFormats[NVMOT_MAX_TRANSFORMS]; /**< Required color formats for input buffers */
    NvBufSurfaceMemType memType;        /**< Preferred memory type of input buffers */
    bool supportBatchProcessing;        /**< Whether batch processing is supported */
} NvMOTQuery;

/**
 * @brief Opaque context struct.
 */
struct NvMOTContext;
typedef struct NvMOTContext* NvMOTContextHandle;

/**
 * @brief Initlize tracking context for a batch
 *
 * Initialize the tracking context for a batch of one or more image streams.
 * If successful, the context is configured per the specified config requests.
 * All future operations on the batch must contain the provided context pointer.
 *
 * @param [int]pConfigIn Pointer to requested configura1tion
 * @param [out] ppContext Pointer to stream context pointer. Stream context to be allocated
 *        and owned by the tracker. The returned context pointer is to be included in
 *        all subsequent calls for the specified stream, until NvMOTDeInit is
 *        called on it.
 * @param [out] pConfigResponse Pointer to configuration status
 * @return The status of the initialization attempt, one of NVTSTATUS_*.
 */
NvMOTStatus NvMOT_Init(NvMOTConfig *pConfigIn,
                       NvMOTContextHandle *pContextHandle,
                       NvMOTConfigResponse *pConfigResponse);

/**
 * @brief De-initialize a stream context
 *
 * The specified context will be retired and never used again.
 *
 * @param pContext Pointer to the stream context to retire.
 */
void NvMOT_DeInit(NvMOTContextHandle contextHandle);

/**
 * @brief Process a batch
 *
 * Given a context and batch of frame(s), process the batch as the current
 * frame(s) in their respective streams. Once processed, each frame becomes part 
 * of the history and previous frame in its stream.
 *
 * @param [in] pContext The context handle obtained from NvMOTInit()
 * @param [in] pParams Pointer to parameters for the batch to be processed
 * @param [out] pTrackedObjBatch Batch of lists of tracked object slots to be filled by the tracker.
                Allocated by the client. BBoxes are scaled to the resolution of the first input image transform buffer.
 * @return Status of batch processing
 */
NvMOTStatus NvMOT_Process(NvMOTContextHandle contextHandle,
                          NvMOTProcessParams *pParams,
                          NvMOTTrackedObjBatch *pTrackedObjectsBatch);

/**
 * @brief Query tracker lib capabilities and requirements.
 *
 * Answer query for this tracker lib's capabilities and requirements.
 * The tracker's custom config file is provided for optional consultation.
 *
 * @param [in] customCnfigFilePathSize Size of the config file path
 * @param [in] pCustomConfigFilePath Pointer to the string containing custom config file path
 * @param [out] pQuery Pointer to the query struct to be filled out by the tracker
 * @return Status of the query
 */
NvMOTStatus NvMOT_Query(uint16_t customConfigFilePathSize, char* pCustomConfigFilePath, NvMOTQuery *pQuery);


/**
 * @brief Optional handler for removing streams from a batch
 *
 * Used in batch processing mode only. This call notifies the tracker lib that
 * a stream is removed, and will not be present in future batches. If there are
 * per-stream resources to be freed, this is an opportunity to do so.
 * This will be called only when all processing is quiesced.
 *
 * @param [in] pContext The context handle obtained from NvMOTInit()
 * @param [in] Mask for finding streams to remove. Remove all streams where
 *             (streamId & streamIdMask) == streamIdMask
 */
void NvMOT_RemoveStreams(NvMOTContextHandle contextHandle, NvMOTStreamId streamIdMask);

/** @} */ // end of API group

#ifdef __cplusplus
}
#endif

#endif
