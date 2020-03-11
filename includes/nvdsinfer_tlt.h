/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

/**
 * @file nvdsinfer_tlt.h
 * <b>NVIDIA DeepStream API for importing Tranfer Learning Toolkit encoded
 * models. </b>
 *
 * @b Description: This file specifies the API to decode and create CUDA engine
 * file from a Tranfer Learning Toolkit(TLT) encoded model.
 */
#ifndef __NVDSINFER_TLT_H__
#define __NVDSINFER_TLT_H__

#include <nvdsinfer_custom_impl.h>

/**
 * API to decode and create CUDA engine file from a TLT encoded model.
 *
 * This API is an implmentation of `NvDsInferCudaEngineGet` interface. The
 * correct key and model path should be provided in the `tltModelKey` and
 * `tltEncodedModelFilePath` members of `initParams`. Other parameters applicable
 * to UFF models also apply to TLT encoded models.
 */
extern "C"
bool NvDsInferCudaEngineGetFromTltModel(nvinfer1::IBuilder *builder,
        NvDsInferContextInitParams *initParams,
        nvinfer1::DataType dataType,
        nvinfer1::ICudaEngine *& cudaEngine);

#endif
