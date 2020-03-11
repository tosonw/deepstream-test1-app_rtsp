/**
 * Copyright (c) 2018-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

/**
 * @file nvdsinfer_context.h
 * <b>NVIDIA DeepStream Inference Interface </b>
 *
 * @b Description: This file specifies the DeepStream inference interface APIs.
 *
 * The DeepStream inference API "NvDsInfer" provides methods to initialize/
 * deinitialize the inference engine, pre-process the input frames as required
 * by the network, and parse the output from the raw tensor buffers.
 *
 * Both C and C++ interfaces are available with the C interface being a simple
 * wrapper over the C++ interface.
 *
 * An opaque handle (NvDsInferContext) to an instance of the context required by
 * the APIs can be created using factory functions createNvDsInferContext /
 * NvDsInferContext_Create. The two functions accept an instance of
 * NvDsInferContextInitParams structure to initialize the context. Logging
 * callbacks can be set to get detailed information about failures/warnings.
 *
 * Initialization parameters allow configuration of the network data type,
 * type of the network(Detector/Classifier/Other), pre-processing parameters
 * (mean subtraction and normalization), model related parameters like Caffe/Uff/Onnx
 * model file paths, output layer names etc.
 *
 * Batches of frames can be queued for inferencing using NvDsInferContext::queueInputBatch /
 * NvDsInferContext_QueueInputBatch. The input frame memories should be accessible
 * by the GPU device configured during initialization. An asynchronous callback
 * function can be provided to return the input buffers back to the caller as
 * soon as the input is consumed.
 *
 * Inference output can be dequeued using NvDsInferContext::dequeueOutputBatch /
 * NvDsInferContext_DequeueOutputBatch. The order dequeued outputs corresponds
 * to the input queueing order. In case of failure, the output of the batch is
 * lost. The dequeued output must be released back to the context using
 * NvDsInferContext::releaseBatchOutput / NvDsInferContext_ReleaseBatchOutput
 * to free the associated memory and return the output layer buffers for re-use
 * by the context.
 *
 * Detectors will output an array of detected objects for each frame in the batch.
 * Classifiers will classify entire frames and output an array of attributes for
 * each frame. Segmentation will classify each pixel in the frame. A special
 * network type has been provided (Other) where the output layers will not be
 * parsed. The caller can parse the device/host output layer buffers. This can
 * also be used along with DeepStream GStreamer Inference plugin "nvinfer" to
 * flow the output buffers as metadata.
 *
 * Other methods to get the parsed labels from a labels file and properties of
 * all the layers bound by the inferece engine have been provided.
 *
 * The NvDsInfer APIs can be extended using the custom method implementations.
 * Refer to the Custom Method Implementations section for more details.
 *
 */
#ifndef __NVDSINFER_CONTEXT_H__
#define __NVDSINFER_CONTEXT_H__

#include "nvdsinfer.h"

/** @name NvDsInferContext API common types and functions.
 *  This section describes the common types and functions for both the C and C++
 *  interfaces for the NvDsInferContext class.
 */

/** @{ */

/** Maximum length of a file path parameter. */
#define _PATH_MAX 4096

/** Maximum number of channels supported by the API for image input layers. */
#define _MAX_CHANNELS 4

/** Maximum length of string parameters. */
#define _MAX_STR_LENGTH 1024

/** Maximum batch size to be supported by nvdsinfer. */
#define NVDSINFER_MAX_BATCH_SIZE 1024

/** Minimum number of sets of output buffers that need to be allocated. */
#define NVDSINFER_MIN_OUTPUT_BUFFERPOOL_SIZE 2

/**
 * Enum for internal data format to be used by the inference engine.
 */
typedef enum
{
    NvDsInferNetworkMode_FP32,
    NvDsInferNetworkMode_INT8,
    NvDsInferNetworkMode_FP16
} NvDsInferNetworkMode;

/**
 * Enum for the type of the network.
 */
typedef enum
{
    /** Detectors will find objects and their coordinates in the input frame along
     * with the class of the object. */
    NvDsInferNetworkType_Detector,
    /** Classifiers - will classify the entire frame into some finite possible classes. */
    NvDsInferNetworkType_Classifier,
    /** Segmentation - will classify each pixel into some finite possible classes. */
    NvDsInferNetworkType_Segmentation,
    /** Other - Output layers will not be parsed by NvDsInferContext. This is useful
     *  for networks producing custom output. The output can be parsed by
     *  the NvDsInferContext client or can be combined with "gst-nvinfer"
     *  feature to flow output tensors as metadata. */
    NvDsInferNetworkType_Other = 100
} NvDsInferNetworkType;

/**
 * Enum for color formats.
 */
typedef enum
{
    /** 24-bit interleaved R-G-B */
    NvDsInferFormat_RGB,
    /** 24-bit interleaved B-G-R */
    NvDsInferFormat_BGR,
    /** 8-bit Luma */
    NvDsInferFormat_GRAY,
    /** 32-bit interleaved R-G-B-A */
    NvDsInferFormat_RGBA,
    /** 32-bit interleaved B-G-R-x */
    NvDsInferFormat_BGRx,
    NvDsInferFormat_Unknown = 0xFFFFFFFF,
} NvDsInferFormat;

/**
 * Enum for the UFF input layer order.
 */
typedef enum
{
    NvDsInferUffInputOrder_kNCHW,
    NvDsInferUffInputOrder_kNHWC,
    NvDsInferUffInputOrder_kNC,
} NvDsInferUffInputOrder;

/**
 * Holds the detection and bounding box grouping parameters.
 */
typedef struct
{
    /** Bounding box detection threshold. */
    float threshold;
    /** Epsilon to control merging of overlapping boxes. Refer to OpenCV
     * groupRectangles and DBSCAN documentation for more information on epsilon */
    float eps;
    /** Minimum boxes in a cluster to be considered an object during
     * grouping using DBSCAN. */
    int minBoxes;
    /** Minimum boxes in a cluster to be considered an object during
     * grouping using OpenCV groupRectangles. */
    int groupThreshold;
} NvDsInferDetectionParams;


/**
 * Enum for the status codes returned by NvDsInferContext.
 */
typedef enum
{
    /** NvDsInferContext operation succeeded. */
    NVDSINFER_SUCCESS = 0,
    /** Failed to configure the NvDsInferContext instance possibly due to an
     *  erroneous initialization property. */
    NVDSINFER_CONFIG_FAILED,
    /** Custom Library interface implementation failed. */
    NVDSINFER_CUSTOM_LIB_FAILED,
    /** Invalid parameters were supplied. */
    NVDSINFER_INVALID_PARAMS,
    /** Output parsing failed. */
    NVDSINFER_OUTPUT_PARSING_FAILED,
    /** CUDA error was encountered. */
    NVDSINFER_CUDA_ERROR,
    /** TensorRT interface failed. */
    NVDSINFER_TENSORRT_ERROR,
    /** Unknown error was encountered. */
    NVDSINFER_UNKNOWN_ERROR
} NvDsInferStatus;

/**
 * Enum for the log levels of NvDsInferContext.
 */
typedef enum
{
    NVDSINFER_LOG_ERROR,
    NVDSINFER_LOG_WARNING,
    NVDSINFER_LOG_INFO,
    NVDSINFER_LOG_DEBUG,
} NvDsInferLogLevel;

/**
 * Holds the initialization parameters required for the NvDsInferContext interface.
 */
typedef struct _NvDsInferContextInitParams
{
    /** Unique identifier for the instance. This can be used to identify the
     * instance generating log and error messages. */
    unsigned int uniqueID;

    /** Internal data format to be used by the inference engine. */
    NvDsInferNetworkMode networkMode;

    /** Path to the prototxt file. */
    char protoFilePath[_PATH_MAX];
    /** Path to the caffemodel file. */
    char modelFilePath[_PATH_MAX];
    /** Path to the UFF model file. */
    char uffFilePath[_PATH_MAX];
    /** Path to the ONNX model file. */
    char onnxFilePath[_PATH_MAX];
    /** Path to the TLT encoded model file. */
    char tltEncodedModelFilePath[_PATH_MAX];

    /** Path to the INT8 calibration file. Required only when using INT8 mode. */
    char int8CalibrationFilePath[_PATH_MAX];

    /** Input dimensions for the UFF model. */
    NvDsInferDimsCHW uffDimsCHW;
    /** Original input order for the UFF model. */
    NvDsInferUffInputOrder uffInputOrder;
    /** Name of the input layer for the UFF model. */
    char uffInputBlobName[_MAX_STR_LENGTH];

    /** String key for decoding the TLT encoded model. */
    char tltModelKey[_MAX_STR_LENGTH];

    /** Path to the serialized model engine file. When using model engine file,
     * other parameters required for creating the model engine are ignored.
     */
    char modelEngineFilePath[_PATH_MAX];

    /** Max number of frames that will be inferred together in a batch. The number
     * of input frames in a batch should be less than or equal to this number.
     */
    unsigned int maxBatchSize;

    /** Path to the labels file containing strings for the class labels. Specifying
     * the labels file is not mandatory. Refer to the custom models section of
     * DeepStreamSDK documentation for the file format.
     */
    char labelsFilePath[_PATH_MAX];

    /** Path to the mean image file (PPM format). Resolution of the file should be
     * equal to the network input resolution.
     */
    char meanImageFilePath[_PATH_MAX];

    /** Normalization factor to scale the input pixels with. */
    float networkScaleFactor;

    /** Network input format. */
    NvDsInferFormat networkInputFormat;

    /** Per channel offsets for mean subtraction. This is an alternative to the
     * mean image file. The number of offsets in the array should be exactly
     * equalto the number of input channels.
     */
    float offsets[_MAX_CHANNELS];
    unsigned int numOffsets;

    /** Type of the network. */
    NvDsInferNetworkType networkType;

    /** Boolean indicating if DBScan should be used for object clustering.
     *  OpenCV groupRectangles will be used if set to false. */
    int useDBScan;

    /** Number of classes detected by a detector network. */
    unsigned int numDetectedClasses;

    /** Per class detection parameters. The size of the array should be equal
     * to numDetectedClasses. */
    NvDsInferDetectionParams *perClassDetectionParams;

    /** Minimum confidence threshold for classifier to consider a label valid. */
    float classifierThreshold;

    float segmentationThreshold;

    /** Array of output Layer Names. */
    char ** outputLayerNames;
    unsigned int numOutputLayers;

    /** Path to the library containing custom methods required to support the
     * network. */
    char customLibPath[_PATH_MAX];
    /** Name of the custom bounding box function in the custom library. */
    char customBBoxParseFuncName[_MAX_STR_LENGTH];
    /** Name of the custom classifier attribute parsing function in the custom
     *  library. */
    char customClassifierParseFuncName[_MAX_STR_LENGTH];

    /** Boolean indicating if input layer contents should be copied to
     * host memories for access in the application. */
    int copyInputToHostBuffers;

    /** ID of the GPU to run the inference on. */
    unsigned int gpuID;

    /** Boolean indicating if DLA should be used. */
    int useDLA;
    /** DLA Core to use. */
    int dlaCore;

    /** Number of sets of output buffers (host and device) to be allocated. */
    unsigned int outputBufferPoolSize;

    /** Path to the config file for custom network creation. This can be used to
     * store custom properties required by the custom network creation function. */
    char customNetworkConfigFilePath[_PATH_MAX];
} NvDsInferContextInitParams;

/**
 * Callback function type for returning the input client buffers back to the
 * NvDsInferContext client asynchronously.
 *
 * @param[in] data The opaque pointer provided to input queueing function
 *            through NvDsInferContextBatchInput.
 */
typedef void (* NvDsInferContextReturnInputAsyncFunc) (void *data);

/**
 * Holds the information about one batch to be inferred.
 */
typedef struct
{
    /** Array of pointers to buffers for input frames. The size of the array
     *  should be at least numInputFrames. */
    void** inputFrames;
    /** Number of input frames i.e. size of the batch. */
    unsigned int numInputFrames;
    /** Format of the frame contents. */
    NvDsInferFormat inputFormat;
    /** Pitch of the input frames, in bytes. */
    unsigned int inputPitch;
    /** Callback function for returning the input buffers back to the client. */
    NvDsInferContextReturnInputAsyncFunc returnInputFunc;
    /** Pointer to the data to be supplied with the return
     *  NvDsInferContextReturnInputAsyncFunc callback. */
    void *returnFuncData;
} NvDsInferContextBatchInput;

/**
 * Holds the information about one detected object.
 */
typedef struct
{
    /** Offset from the left boundary of the frame. */
    unsigned int left;
    /** Offset from the top boundary of the frame. */
    unsigned int top;
    /** Object width. */
    unsigned int width;
    /** Object height. */
    unsigned int height;
    /* Index for the object class. */
    int classIndex;
    /* String label for the detected object. */
    char *label;
} NvDsInferObject;

/**
 * Holds the information on all objects detected by a detector network in one
 * frame.
 */
typedef struct
{
    /** Array of objects. */
    NvDsInferObject *objects;
    /** Number of objects in the array. */
    unsigned int numObjects;
} NvDsInferDetectionOutput;

/**
 * Holds the information on all attributes classifed by a classifier network for
 * one frame.
 */
typedef struct
{
    /** Array of attributes. Maybe more than one depending on the number of
     * output coverage layers (multi-label classifiers) */
    NvDsInferAttribute *attributes;
    /** Size of the attributes array. */
    unsigned int numAttributes;
    /** String label for the classified output. */
    char *label;
} NvDsInferClassificationOutput;

/**
 * Holds the information parsed from segmentation network output for
 * one frame.
 */
typedef struct
{
    /** Width of the output. Same as network width. */
    unsigned int width;
    /** Height of the output. Same as network height. */
    unsigned int height;
    /** Number of classes supported by the network. */
    unsigned int classes;
    /** Pointer to the array for 2D pixel class map. The output for pixel (x,y)
     * will be at index (y * width + x). */
    int *class_map;
    /** Pointer to the raw array containing the probabilities. The probability for
     * class c and pixel (x,y) will be at index (c * width *height + y * width + x). */
    float *class_probability_map;
} NvDsInferSegmentationOutput;

/**
 * Holds the information inferred by the network on one frame.
 */
typedef struct
{
    /** Output type indicating the valid member in the union. This is basically
     * the network type. */
    NvDsInferNetworkType outputType;
    /** Union of the various outputs supported. Valid member is based on the
     * `outputType`. */
    union
    {
        /** Detector output. Valid when outputType is NvDsInferNetworkType_Detector. */
        NvDsInferDetectionOutput detectionOutput;
        /** Classifier output. Valid when outputType is NvDsInferNetworkType_Classifier. */
        NvDsInferClassificationOutput classificationOutput;
        /** Classifier output. Valid when outputType is NvDsInferNetworkType_Classifier. */
        NvDsInferSegmentationOutput segmentationOutput;
    };
} NvDsInferFrameOutput;

/**
 * A batch is an array of frames. This structure holds the output for
 * all the frames in the batch and related buffers information.
 */
typedef struct
{
    /** Array of outputs for each frame in the batch. */
    NvDsInferFrameOutput *frames;
    unsigned int numFrames;

    /** Array of pointers to set of output device buffers for this batch. */
    void **outputDeviceBuffers;
    unsigned int numOutputDeviceBuffers;

    /** Array of pointers to set of host buffers for this batch. */
    void **hostBuffers;
    unsigned int numHostBuffers;

    /** ID for the set of output buffers. */
    unsigned int outputBatchID;
} NvDsInferContextBatchOutput;

/** An opaque pointer type to be used as an handle for the context instance. */
typedef struct INvDsInferContext * NvDsInferContextHandle;

/** Callback function type for logging the NvDsInferContext messages.
 *
 * @param[in] handle Handle of the NvDsInferContext instance which generated the
 *            log.
 * @param[in] uniqueID UniqueID of the NvDsInferContext instance which generated
 *            the log.
 * @param[in] logLevel Level of the log.
 * @param[in] funcName Name of the function which generated the log.
 * @param[in] logMessage The log message string.
 * @param[in] userCtx The opaque user context pointer supplied when creating the
 *            NvDsInferContext instance.
 */
typedef void (*NvDsInferContextLoggingFunc) (NvDsInferContextHandle handle,
        unsigned int uniqueID, NvDsInferLogLevel logLevel, const char *funcName,
        const char * logMessage, void *userCtx);


#ifdef __cplusplus
extern "C" {
#endif

/** Reset the members of initialization parameters to default values.
 *
 * @param[in] initParams Pointer to a NvDsInferContextInitParams structure.
 */
void NvDsInferContext_ResetInitParams (NvDsInferContextInitParams *initParams);

/** Get the string name for the status.
 *
 * @param[in] status An NvDsInferStatus value.
 * @return String name for the status. NULL in case of an unrecognized status
 *         value. Memory is owned by the function. Callers should not free the
 *         pointer.
 */
const char * NvDsInferContext_GetStatusName (NvDsInferStatus status);


#ifdef __cplusplus
}
#endif

/** @} */

/** @name NvDsInferContext API C++-interface
 *  This section describes the C++ interface for the NvDsInferContext class.
 */

/** @{ */
#ifdef __cplusplus

#include <string>
#include <vector>

/**
 * The DeepStream inference interface class.
 */
struct INvDsInferContext
{
public:
    /**
     * Queue a batch of input frames for pre-processing and inferencing. The input
     * frames are expected to be of packed RGB/RGBA/GRAY UINT8 format with the same
     * resolution as the network input or pre-processed inputs that can be
     * directly fed to the inference engine. The frame memories should be CUDA
     * device memory allocated on the same device as the NvDsInferContext interface
     * is configured with.
     *
     * The batch size should not exceed the max batch size requested during
     * initialization.
     *
     * @param[in] batchInput Reference to a batch input structure.
     * @return NVDSINFER_SUCCESS if pre-processing and queueing succeeded or a
     *         relevant error status.
     */
    virtual NvDsInferStatus queueInputBatch(NvDsInferContextBatchInput &batchInput) = 0;

    /**
     * Dequeue output for a batch of frames. The batch dequeuing order is same
     * as the input queuing order. The associated memory must be freed and output
     * buffers must be released back to the context using releaseBatchOutput so
     * that the buffers can be reused.
     *
     * @param[out] batchOutput Reference to the Batch output structure in which the output
     *                    will be appended.
     * @return NVDSINFER_SUCCESS if dequeueing succeeded or a relevant error status.
     */
    virtual NvDsInferStatus dequeueOutputBatch(NvDsInferContextBatchOutput &batchOutput) = 0;

    /**
     * Free the memory associated with the batch output and release the set of
     * output buffers back to the context for re-use.
     *
     * @param[in] batchOutput Reference to a batchOutput structure which was filled
     *                    by dequeueOutputBatch.
     */
    virtual void releaseBatchOutput(NvDsInferContextBatchOutput &batchOutput) = 0;

    /**
     * Fill the input vector with information on all the bound layers of the
     * inference engine.
     *
     * @param[in,out] layersInfo Reference to a vector of NvDsInferLayerInfo which will
     *                   be filled by the function.
     */
    virtual void fillLayersInfo(std::vector<NvDsInferLayerInfo> &layersInfo) = 0;

    /**
     * Get the network input information.
     *
     * @param[in,out] networkInfo Reference to a NvDsInferNetworkInfo structure.
     */
    virtual void getNetworkInfo(NvDsInferNetworkInfo &networkInfo) = 0;

    /**
     * Get the label strings parsed from the labels file.
     *
     * Refer to the DeepStream NvInfer documentation for the format of the
     * labels file for detectors and classifiers.
     *
     * @return Reference to vector of vector of string labels.
     */
    virtual const std::vector< std::vector<std::string> >& getLabels() = 0;

    /**
     * Free up resources and deinitialize the inference engine.
     */
    virtual void destroy() = 0;
protected:
    virtual ~INvDsInferContext() {}
};

/**
 * Creates a new instance of NvDsInferContext initialized using the supplied
 * parameters.
 *
 * @param[out] handle Pointer to a NvDsInferContext handle.
 * @param[in]  initParams Parameters to use for initialization of the context.
 * @param[in]  userCtx Pointer to an opaque user context supplied with callbacks
 *             generated by the NvDsInferContext instance.
 * @param[in] logFunc Log callback function for the instance.
 * @return NVDSINFER_SUCCESS if the creation succeeded or a relevant error status.
 */
NvDsInferStatus createNvDsInferContext(NvDsInferContextHandle *handle,
        NvDsInferContextInitParams &initParams,
        void *userCtx = nullptr,
        NvDsInferContextLoggingFunc logFunc = nullptr);

#endif

/** @} */

/** @name NvDsInferContext API C-interface
 *  This section describes the C interface for the NvDsInferContext class.
 */

/** @{ */

#ifdef __cplusplus
extern "C" {
#endif


/** Creates a new instance of the NvDsInferContext class with the supplied
 *  initialization parameters.
 *
 * @param[out] handle Pointer to a NvDsInferContext handle.
 * @param[in]  initParams Parameters to use for initialization of the context.
 * @param[in]  userCtx Pointer to an opaque user context supplied with callbacks
 *             generated by the NvDsInferContext instance.
 * @param[in] logFunc Log callback function for the instance.
 * @return NVDSINFER_SUCCESS if the creation succeeded or a relevant error status.
 */
NvDsInferStatus NvDsInferContext_Create(NvDsInferContextHandle *handle,
        NvDsInferContextInitParams *initParams, void *userCtx,
        NvDsInferContextLoggingFunc logFunc);

/** Destroy a NvDsInferContext instance and release the associated resources.
 *
 * @param[in] handle Handle to the NvDsInferContext instance to be destroyed.
 */
void NvDsInferContext_Destroy (NvDsInferContextHandle handle);

/**
 * Queue a batch of input frames for pre-processing and inferencing.
 *
 * Refer NvDsInferContext::queueInputBatch() for details.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in] batchInput Reference to a batch input structure.
 * @return NVDSINFER_SUCCESS if pre-processing and queueing succeeded or a
 *         relevant error status.
 */
NvDsInferStatus NvDsInferContext_QueueInputBatch(NvDsInferContextHandle handle,
        NvDsInferContextBatchInput *batchInput);

/**
 * Dequeue output for a batch of frames.
 *
 * Refer NvDsInferContext::dequeueOutputBatch() for details.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in,out] batchOutput Reference to the Batch output structure in which
 *                the output will be appended.
 * @return NVDSINFER_SUCCESS if dequeueing succeeded or a relevant error status.
 */
NvDsInferStatus NvDsInferContext_DequeueOutputBatch(NvDsInferContextHandle handle,
        NvDsInferContextBatchOutput *batchOutput);

/**
 * Free the memory associated with the batch output and release the set of
 * host buffers back to the context for re-use.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in] batchOutput Pointer to a NvDsInferContext_ReleaseBatchOutput structure
 *                    which was filled by NvDsInferContext_DequeueOutputBatch().
 */
void NvDsInferContext_ReleaseBatchOutput(NvDsInferContextHandle handle,
        NvDsInferContextBatchOutput *batchOutput);

/**
 * Get the network input information.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in,out] networkInfo Pointer to a NvDsInferNetworkInfo structure.
 */
void NvDsInferContext_GetNetworkInfo(NvDsInferContextHandle handle,
        NvDsInferNetworkInfo *networkInfo);

/**
 * Get the number of the bound layers of the inference engine in the
 * NvDsInferContext instance.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @return The number of the bound layers of the inference engine.
 */
unsigned int NvDsInferContext_GetNumLayersInfo(NvDsInferContextHandle handle);

/**
 * Fill the input vector with information on all the bound layers of the
 * inference engine in the NvDsInferContext instance. The size of the array has to
 * be at least the value returned by NvDsInferContext_GetNumLayersInfo.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in,out] layersInfo Pointer to an array of NvDsInferLayerInfo which will
 *                   be filled by the function.
 */
void NvDsInferContext_FillLayersInfo(NvDsInferContextHandle handle,
        NvDsInferLayerInfo *layersInfo);

/**
 * Get the string label associated with the class_id for detectors and the
 * attribute id and the attribute value for classifiers. The string is owned
 * by the context. Callers should not modify or free the string.
 *
 * @param[in] handle Handle to a NvDsInferContext instance.
 * @param[in] id Class ID for detectors, Attribute ID for classifiers.
 * @param[in] value Attribute value for classifiers, should be 0 for detectors.
 * @return Pointer to a string label. The memory is owned by the context.
 */
const char* NvDsInferContext_GetLabel(NvDsInferContextHandle handle,
        unsigned int id, unsigned int value);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
