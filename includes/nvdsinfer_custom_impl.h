/**
 * Copyright (c) 2017-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

/**
 * @file nvdsinfer_custom_impl.h
 * <b>NVIDIA DeepStream Specification for Custom Method Implementations for custom models </b>
 *
 * @b Description: This file specifies the APIs and function definitions to
 * implement custom methods required by DeepStream nvinfer GStreamer plugin to
 * infer using custom models.
 *
 * All the custom functionality must be implemented in an independent shared
 * library. This library will be dynamically loaded (dlopen()) by "nvinfer" and
 * implemented custom methods will be called as required. The custom library can
 * be specified in the nvinfer element configuration file through the
 * `custom-lib-name` property.
 *
 * @section customparsingfunc Custom Detector Output Parsing Function
 *
 * This section describes the custom bounding box parsing function for custom
 * detector models.
 *
 * The custom parsing function should be of the type `NvDsInferParseCustomFunc`.
 * The custom parsing function can be specified in the nvinfer element
 * configuration file using the property `parse-bbox-func-name`
 * (Name of the parsing function) in addition to `custom-lib-name` and setting
 * `parse-func` to 0 (custom).
 *
 * The nvinfer plugin loads the library and looks for the custom parsing
 * function symbol. The function is called after each inference call is
 * executed.
 *
 * The macro CHECK_CUSTOM_PARSE_FUNC_PROTOTYPE() can be called after the function
 * definition to validate the function definition.
 *
 *
 * @section iplugininterface TensorRT Plugin Factory interface for DeepStream
 *
 * Based on the type of the model (Caffe or UFF), the library
 * must implement one of the two functions NvDsInferPluginFactoryCaffeGet or
 * NvDsInferPluginFactoryUffGet.
 * During model parsing, the DeepStream "nvinfer" plugin will look for one of
 * the two symbols in the custom library based on the model framework. If the
 * symbol is found, the plugin will call the "Get" function to get a pointer to
 * PluginFactory instance required for parsing.
 *
 * If the IPluginFactory needs to be used during deserialization of cuda engines,
 * the library must implement NvDsInferPluginFactoryRuntimeGet.
 *
 * All the three Get functions have a corresponding Destroy functions which will
 * be called if defined when the returned PluginFactory need to be destroyed.
 *
 * Libraries implementing this interface must use the same function names as in
 * the header file. The "nvinfer" plugin will dynamically load the library and
 * look for the same symbol names.
 *
 * Refer the FasterRCNN sample provided with the SDK for a sample implementation
 * of the interface.
 *
 *
 * @section inputlayerinitialization Input Layers Initialization
 *
 * By default "nvinfer" works with networks having only one input layer for video
 * frames. If the network has more than one input layer, the custom library
 * can implement NvDsInferInitializeInputLayers interface for initializing the
 * other input layers. "nvinfer" assumes that the other input layers have static
 * input information and hence this method is called only once before the first
 * inference.
 *
 * Refer the FasterRCNN sample provided with the SDK for a sample implementation
 * of the interface.
 *
 *
 * @section customnetworkbuild Interface for building Custom Networks
 *
 * The NvDsInferCudaEngineGet interface can be used to create and build custom
 * networks not directly supported by nvinfer.
 *
 * The "nvinfer" plugin will dynamically load the custom library and look for the
 * "NvDsInferCudaEngineGet" symbol.
 *
 * The interface implementation shall build and return the CudaEngine interface
 * using the supplied nvinfer1::IBuilder instance. The builder instance will
 * already be configured with properties like MaxBatchSize, MaxWorkspaceSize,
 * INT8/FP16 precision parameters etc. The builder instance is managed by "nvinfer"
 * and should not be destroyed by the implementation.
 *
 * The path to the config file for nvinfer instance is supplied to the interface.
 * Custom properties required by the model can be added to the config file and
 * parsed in the interface implementation.
 *
 */

#ifndef _NVDSINFER_CUSTOM_IMPL_H_
#define _NVDSINFER_CUSTOM_IMPL_H_

#include <string>
#include <vector>
#include "NvCaffeParser.h"
#include "NvUffParser.h"

#include "nvdsinfer.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Holds the detection parameters required for parsing objects.
 */
typedef struct
{
  /** Number of classes requested to be parsed starting with classID 0.
   *  Parsing functions should only output objects with classID < numClassesConfigured. */
  unsigned int numClassesConfigured;
  /** Per class detection confidence threshold. Parsing functions should only
   * output objects with detectionConfidence >= perClassThreshold[object's classID] */
  std::vector<float> perClassThreshold;
} NvDsInferParseDetectionParams;

/**
 * Function definition for the custom bounding box parsing function.
 *
 * @param[in]  outputLayersInfo Vector containing information on the output
 *            layers of the model.
 * @param[in]  networkInfo Network information.
 * @param[in]  detectionParams Detection parameters required for parsing objects.
 * @param[out] objectList Reference to a vector in which the function should add
 *             the parsed objects.
 */
typedef bool (* NvDsInferParseCustomFunc) (
        std::vector<NvDsInferLayerInfo> const &outputLayersInfo,
        NvDsInferNetworkInfo  const &networkInfo,
        NvDsInferParseDetectionParams const &detectionParams,
        std::vector<NvDsInferObjectDetectionInfo> &objectList);

/**
 * Macro to validate the custom parser function definition. Should be called
 * after defining the function.
 */
#define CHECK_CUSTOM_PARSE_FUNC_PROTOTYPE(customParseFunc) \
    static void checkFunc_ ## customParseFunc (NvDsInferParseCustomFunc func = customParseFunc) \
        { checkFunc_ ## customParseFunc (); }; \
    extern "C" bool customParseFunc (std::vector<NvDsInferLayerInfo> const &outputLayersInfo, \
           NvDsInferNetworkInfo  const &networkInfo, \
           NvDsInferParseDetectionParams const &detectionParams, \
           std::vector<NvDsInferObjectDetectionInfo> &objectList);

/**
 * Function definition for the custom classifier output parsing function.
 *
 * @param[in]  outputLayersInfo Vector containing information on the output
 *            layers of the model.
 * @param[in]  networkInfo Network information.
 * @param[in]  classifierThreshold Classification confidence threshold
 * @param[out] attrList Reference to a vector in which the function should add
 *             the parsed attributes.
 * @param[out] descString Reference to a string object in which the function
 *             may place a description string.
 */
typedef bool (* NvDsInferClassiferParseCustomFunc) (
        std::vector<NvDsInferLayerInfo> const &outputLayersInfo,
        NvDsInferNetworkInfo  const &networkInfo,
        float classifierThreshold,
        std::vector<NvDsInferAttribute> &attrList,
        std::string &descString);

/**
 * Macro to validate the classifier custom parser function definition. Should be called
 * after defining the function.
 */
#define CHECK_CUSTOM_CLASSIFIER_PARSE_FUNC_PROTOTYPE(customParseFunc) \
    static void checkFunc_ ## customParseFunc (NvDsInferClassiferParseCustomFunc func = customParseFunc) \
        { checkFunc_ ## customParseFunc (); }; \
    extern "C" bool customParseFunc (std::vector<NvDsInferLayerInfo> const &outputLayersInfo, \
           NvDsInferNetworkInfo  const &networkInfo, \
           float classifierThreshold, \
           std::vector<NvDsInferAttribute> &attrList, \
           std::string &descString);

/**
 * Specifies the type of the Plugin Factory.
 */
typedef enum
{
  /** nvcaffeparser1::IPluginFactory / nvuffparser::IPluginFactory */
  PLUGIN_FACTORY,
  /** nvcaffeparser1::IPluginFactoryExt / nvuffparser::IPluginFactoryExt */
  PLUGIN_FACTORY_EXT,
  /** Only for caffe models. nvcaffeparser1::IPluginFactoryV2 */
  PLUGIN_FACTORY_V2
} NvDsInferPluginFactoryType;

/**
 * Holds the pointer to a heap allocated Plugin Factory object required during
 * Caffemodel parsing.
 */
typedef union
{
  nvcaffeparser1::IPluginFactory *pluginFactory;
  nvcaffeparser1::IPluginFactoryExt *pluginFactoryExt;
  nvcaffeparser1::IPluginFactoryV2 *pluginFactoryV2;
} NvDsInferPluginFactoryCaffe;

/**
 * Holds the pointer to a heap allocated Plugin Factory object required during
 * UFF model parsing.
 */
typedef union
{
  nvuffparser::IPluginFactory *pluginFactory;
  nvuffparser::IPluginFactoryExt *pluginFactoryExt;
} NvDsInferPluginFactoryUff;

/**
 * Returns an instance of a newly allocated Plugin Factory interface to be used
 * during parsing of caffe models. The function must set the correct `type` and
 * the correct field inside the `NvDsInferPluginFactoryCaffe` union based on the
 * type of the Plugin Factory (i.e. one of pluginFactory, pluginFactoryExt or
 * pluginFactoryV2).
 *
 * @param[out] pluginFactory A reference to the `NvDsInferPluginFactoryCaffe` union
 * @param[out] type Enum indicating the type of the returned Plugin Factory
 *             instance (i.e. the valid field in the `NvDsInferPluginFactoryCaffe`
 *             union)
 *
 * @return Boolean indicating that the creation of the Plugin Factory was
 *        successful.
 */
bool NvDsInferPluginFactoryCaffeGet (NvDsInferPluginFactoryCaffe &pluginFactory,
    NvDsInferPluginFactoryType &type);

/**
 * Destroy the Plugin Factory instance returned in `NvDsInferPluginFactoryCaffeGet`
 *
 * @param[in] pluginFactory Reference to `NvDsInferPluginFactoryCaffe` union holding
 *            the pointer to the Plugin Factory instance returned in
 *            `NvDsInferPluginFactoryCaffeGet`.
 */
void NvDsInferPluginFactoryCaffeDestroy (NvDsInferPluginFactoryCaffe &pluginFactory);

/**
 * Returns an instance of a newly allocated Plugin Factory interface to be used
 * during parsing of UFF models. The function must set the correct `type` and
 * the correct field inside the `NvDsInferPluginFactoryUff` union based on the
 * type of the Plugin Factory (i.e. one of pluginFactory or pluginFactoryExt)
 *
 * @param[out] pluginFactory A reference to the `NvDsInferPluginFactoryUff` union
 * @param[out] type Enum indicating the type of the returned Plugin Factory
 *             instance (i.e. the valid field in the `NvDsInferPluginFactoryUff`
 *             union)
 *
 * @return Boolean indicating that the creation of the Plugin Factory was
 *        successful.
 */
bool NvDsInferPluginFactoryUffGet (NvDsInferPluginFactoryUff &pluginFactory,
    NvDsInferPluginFactoryType &type);

/**
 * Destroy the Plugin Factory instance returned in `NvDsInferPluginFactoryUffGet`
 *
 * @param[in] pluginFactory Reference to `NvDsInferPluginFactoryUff` union holding
 *            the pointer to the Plugin Factory instance returned in
 *            `NvDsInferPluginFactoryUffGet`.
 */
void NvDsInferPluginFactoryUffDestroy (NvDsInferPluginFactoryUff &pluginFactory);

/**
 * Returns an instance of a newly allocated Plugin Factory interface to be used
 * during parsing deserialization of cuda engines.
 *
 * @param[out] pluginFactory A reference to nvinfer1::IPluginFactory* in which
 *             the function must place the pointer to the instance.
 *
 * @return Boolean indicating that the creation of the Plugin Factory was
 *        successful.
 */
bool NvDsInferPluginFactoryRuntimeGet (nvinfer1::IPluginFactory *& pluginFactory);

/**
 * Destroy the PluginFactory instance returned in `NvDsInferPluginFactoryRuntimeGet`
 *
 * @param[in] pluginFactory PluginFactory instance returned in
 *            `NvDsInferPluginFactoryRuntimeGet`
 */
void NvDsInferPluginFactoryRuntimeDestroy (nvinfer1::IPluginFactory * pluginFactory);

/**
 * Initialize the input layers for inference. This function is only called once
 * during before the first inference call.
 *
 * @param[in] inputLayersInfo Vector containing information on the input layers
 *            of the model. This does not contain the NvDsInferLayerInfo
 *            structure for the layer for video frame input.
 * @param[in] networkInfo Network information.
 * @param[in] maxBatchSize Maximum batch size for inference. The buffers for
 *            input layers are allocated for this batch size.
 *
 * @return Boolean indicating that input layers were successfully initialized.
 */
bool NvDsInferInitializeInputLayers (std::vector<NvDsInferLayerInfo> const &inputLayersInfo,
        NvDsInferNetworkInfo const &networkInfo,
        unsigned int maxBatchSize);

typedef struct _NvDsInferContextInitParams NvDsInferContextInitParams;

/**
 * Build and return CudaEngine for custom models.
 *
 * The implementation of this interface should build the nvinfer1::ICudaEngine
 * instance using supplied nvinfer1::IBuilder instance. The builder instance is
 * managed by the caller. The implementation should not destroy the instance.
 *
 * Properties like MaxBatchSize, MaxWorkspaceSize, INT8/FP16 precision parameters,
 * DLA parameters if applicable will already be set on the builder before being
 * passed to the interface. The corresponding Get functions of nvinfer1::IBuilder
 * interface can be used to get the property values.
 *
 * The implementation should make sure not to reduce the MaxBatchSize of the
 * returned CudaEngine.
 *
 * @param[in] builder nvinfer1::IBuilder instance
 * @param[in] initParams Pointer to the structure used for initializing the
 *            NvDsInferContext instance.
 * @param[in] dataType Data precision
 * @param[out] cudaEngine Pointer to the built nvinfer1::ICudaEngine instance.
 *
 * @return Boolean indicating that engine build was successful.
 */
bool NvDsInferCudaEngineGet(nvinfer1::IBuilder *builder,
        NvDsInferContextInitParams *initParams,
        nvinfer1::DataType dataType,
        nvinfer1::ICudaEngine *& cudaEngine);

#ifdef __cplusplus
}
#endif

#endif

