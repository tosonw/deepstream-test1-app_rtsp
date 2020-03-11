/*
 * Copyright (c) 2018, NVIDIA CORPORATION.  All rights reserved.
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
 * <b>NVIDIA DeepStream: Metadata Extension Structures</b>
 *
 * @b Description: This file specifies the NVIDIA DeepStream Metadata structures used to
 * describe metadata objects.
 */

#ifndef NVDSMETA_H_
#define NVDSMETA_H_

#include <glib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Event type flags.
 */
typedef enum NvDsEventType {
  NVDS_EVENT_ENTRY,
  NVDS_EVENT_EXIT,
  NVDS_EVENT_MOVING,
  NVDS_EVENT_STOPPED,
  NVDS_EVENT_EMPTY,
  NVDS_EVENT_PARKED,
  NVDS_EVENT_RESET,
  /** Reserved for future use. Use value greater than this for custom events. */
  NVDS_EVENT_RESERVED = 0x100,
  /** To support custom event */
  NVDS_EVENT_CUSTOM = 0x101,
  NVDS_EVENT_FORCE32 = 0x7FFFFFFF
} NvDsEventType;

/**
 * Object type flags.
 */
typedef enum NvDsObjectType {
  NVDS_OBJECT_TYPE_VEHICLE,
  NVDS_OBJECT_TYPE_PERSON,
  NVDS_OBJECT_TYPE_FACE,
  NVDS_OBJECT_TYPE_BAG,
  NVDS_OBJECT_TYPE_BICYCLE,
  NVDS_OBJECT_TYPE_ROADSIGN,
  /** Reserved for future use. Use value greater than this for custom objects. */
  NVDS_OBJECT_TYPE_RESERVED = 0x100,
  /** To support custom object. */
  NVDS_OBJECT_TYPE_CUSTOM = 0x101,
  NVDS_OBJECT_TYPE_UNKNOWN = 0x102, /**< "object" key will be missing in the schema */
  NVDS_OBEJCT_TYPE_FORCE32 = 0x7FFFFFFF
} NvDsObjectType;

/**
 * Payload type flags.
 */
typedef enum NvDsPayloadType {
  NVDS_PAYLOAD_DEEPSTREAM,
  NVDS_PAYLOAD_DEEPSTREAM_MINIMAL,
  /** Reserved for future use. Use value greater than this for custom payloads. */
  NVDS_PAYLOAD_RESERVED = 0x100,
  /** To support custom payload. User need to implement nvds_msg2p_* interface */
  NVDS_PAYLOAD_CUSTOM = 0x101,
  NVDS_PAYLOAD_FORCE32 = 0x7FFFFFFF
} NvDsPayloadType;

/**
 * Holds rectangle parameters.
 */
typedef struct NvDsRect {
  gint top;
  gint left;
  gint width;
  gint height;
} NvDsRect;

/**
 * Holds Geo-location parameters.
 */
typedef struct NvDsGeoLocation {
  gdouble lat;
  gdouble lon;
  gdouble alt;
} NvDsGeoLocation;

/**
 * Hold coordinate parameters.
 */
typedef struct NvDsCoordinate {
  gdouble x;
  gdouble y;
  gdouble z;
} NvDsCoordinate;

/**
 * Holds object signature.
 */
typedef struct NvDsObjectSignature {
  /** array of signature values. */
  gdouble *signature;
  /** size of array */
  guint size;
} NvDsObjectSignature;

/**
 * Holds vehicle object parameters.
 */
typedef struct NvDsVehicleObject {
  gchar *type;
  gchar *make;
  gchar *model;
  gchar *color;
  gchar *region;
  gchar *license;
} NvDsVehicleObject;

/**
 * Holds person object parameters.
 */
typedef struct NvDsPersonObject {
  gchar *gender;
  gchar *hair;
  gchar *cap;
  gchar *apparel;
  guint age;
} NvDsPersonObject;

/**
 * Holds face parameters.
 */
typedef struct NvDsFaceObject {
  gchar *gender;
  gchar *hair;
  gchar *cap;
  gchar *glasses;
  gchar *facialhair;
  gchar *name;
  gchar *eyecolor;
  guint age;
} NvDsFaceObject;

/**
 * Holds event message meta data.
 *
 * Various types of objects (e.g. Vehicle, Person, Face etc.) can be
 * attached by allocating that object and setting @a extMsg pointer.
 *
 * Similarly custom object can also be allocated and set to @a extMsg
 * and should be handled by meta data parsing module accordingly.
 */
typedef struct NvDsEventMsgMeta {
  /** type of event */
  NvDsEventType type;
  /** type of object */
  NvDsObjectType objType;
  /** bounding box of object */
  NvDsRect bbox;
  /** Geo-location of object */
  NvDsGeoLocation location;
  /** coordinate of object */
  NvDsCoordinate coordinate;
  /** signature of object */
  NvDsObjectSignature objSignature;
  /** class id of object */
  gint objClassId;
  /** id of sensor that generated the event */
  gint sensorId;
  /** id of analytics module that generated the event */
  gint moduleId;
  /** id of place related to the object */
  gint placeId;
  /** id of component that generated this event */
  gint componentId;
  /** video frame id of this event */
  gint frameId;
  /** confidence of inference */
  gdouble confidence;
  /** tracking id of object */
  gint trackingId;
  /** time stamp of generated event */
  gchar *ts;
  /** label of detected / inferred object */
  gchar *objectId;
  /** Identity string of sensor */
  gchar *sensorStr;
  /** other attributes associated with the object */
  gchar *otherAttrs;
  /** name of video file */
  gchar *videoPath;
  /**
   * To extend the event message meta data.
   * This can be used for custom values that can't be accommodated
   * in the existing fields OR if object(vehicle, person, face etc.) specific
   * values needs to be attached.
   */
  gpointer extMsg;
  /** size of custom object */
  guint extMsgSize;
} NvDsEventMsgMeta;

/**
 * Holds event information.
 */
typedef struct _NvDsEvent {
  /** type of event */
  NvDsEventType eventType;
  /** pointer of event meta data. */
  NvDsEventMsgMeta *metadata;
} NvDsEvent;

/**
 * Holds payload meta data.
 */
typedef struct NvDsPayload {
  /** pointer to payload */
  gpointer payload;
  /** size of payload */
  guint payloadSize;
  /** id of component who attached the payload (Optional) */
  guint componentId;
} NvDsPayload;

#ifdef __cplusplus
}
#endif
#endif /* NVDSMETA_H_ */
