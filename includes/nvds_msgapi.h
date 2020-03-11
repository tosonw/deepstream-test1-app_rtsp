/*
 * Copyright (c) 2018-2019 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */
/*
 * This file defines the NvDS_MsgApi interface.
 * The interfaces is used by DeepStream applications to send and receive
 * messages from remote entities and services to deliver events, allow
 * configuration of settings etc.
 */

#ifndef __NVDS_MSGAPI_H__
#define __NVDS_MSGAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

/**
 * Defines the handle to be used with all methods in the NvDS_MsgApi interface
 */
typedef void *NvDsMsgApiHandle;

/**
 * Defines various events associated with connections to remote entities
 */
typedef enum {
  /** disconnection of connection handle */
  NVDS_MSGAPI_EVT_DISCONNECT,
  /** remote service is down */
  NVDS_MSGAPI_EVT_SERVICE_DOWN 
} NvDsMsgApiEventType;

/**
 * Defines completion status for operations in the NvDS_MsgApi interface
 */
typedef enum {
NVDS_MSGAPI_OK,
NVDS_MSGAPI_ERR,
NVDS_MSGAPI_UNKNOWN_TOPIC
} NvDsMsgApiErrorType;

/**
  * Type definition for send method callback
  *
  * @param[in] user_ptr Pointer passed during async_send for context
  * @param[in] completion_flag Completion status of send operation.
  */
typedef void (*nvds_msgapi_send_cb_t)(void *user_ptr,  NvDsMsgApiErrorType completion_flag);

/**
 * Type definition for handle method callback registered during connect.
 * using which events corresponding to connection are delivered
 *
 * @param[in] h_ptr Handle corresponding to event
 * @param[in] ds_evt Event type based on  enum
 */
typedef void (*nvds_msgapi_connect_cb_t)(NvDsMsgApiHandle h_ptr, NvDsMsgApiEventType ds_evt);

/**
  * Connects to a remote agent by calling into protocol adapter.
  *
  * @param[in] connection_str  A connection string with format
  *                            `"url;port;topic"`.
  * @param[in] connect_cb      A pointer to a callback function for events
  *                            associated with the connection.
  * @param[in] config_path     Pathname of a configuration file passed to
  *                            the protocol adapter.
  *
  * @return A connection handle.
 */
NvDsMsgApiHandle nvds_msgapi_connect(char *connection_str, nvds_msgapi_connect_cb_t connect_cb, char *config_path);

 /**
  * Send message over connection synchronously based on blocking semantics
  *
  * @param[in] h_ptr connection handle
  * @param[in] topic topic to which send message
  * @param[in] payload message data
  * @param[in] nbuf number of bytes of data to send
  *
  * @return Completion status of send operation
 */
NvDsMsgApiErrorType nvds_msgapi_send(NvDsMsgApiHandle h_ptr, char *topic, const uint8_t *payload, size_t nbuf);

 /**
  * Send message over connection asynchronously based on non-blocking semantics
  *
  * @param[in] h_ptr connection handle
  * @param[in] topic topic to which send message
  * @param[in] payload message data
  * @param[in] nbuf number of bytes of data to send
  * @param[in] send_callback callback to be invoked when operation complets
  * @param[in] user_ptr pointer to pass to callback for context
  *
  * @return Completion status of send operation
 */
NvDsMsgApiErrorType nvds_msgapi_send_async(NvDsMsgApiHandle h_ptr, char  *topic, const uint8_t *payload, size_t nbuf, nvds_msgapi_send_cb_t send_callback, void *user_ptr);

/**
 * Calls into the adapter to allow for execution of undnerlying protocol logic.
 * As part of this routine, adapter should service outstanding incoming and
 * outgoing. It can also perform periodic housekeeping tasks such s sending
 * heartbeats.
 * The method makes client control when the protocol logic gets executed.
 * It should be called periodically by the client, as required by the adapter.
 *
 * @param[in] h_ptr connection handle
 */
void nvds_msgapi_do_work(NvDsMsgApiHandle h_ptr);

 /**
  * Terminates existing connection.
  *
  * @param[in] h_ptr connection handle
  *
  * @return Result of termination
 */
NvDsMsgApiErrorType nvds_msgapi_disconnect(NvDsMsgApiHandle h_ptr);

/**
 * Current version of NvDS_MsgApi interface supported by protocol adapter
 *
 * @return The version of the interface supported by the adapter based on
 *  MAJOR.MINOR format.
 */
char *nvds_msgapi_getversion(void);

#ifdef __cplusplus
}
#endif

#endif

