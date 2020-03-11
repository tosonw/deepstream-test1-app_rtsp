/*
 * Copyright (c) 2018 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */
#ifndef NVDS_LOGGER_H
#define NVDS_LOGGER_H

#include <syslog.h>

#define DSLOG_SYSLOG_IDENT "DSLOG"

//define additional categories here
#define DSLOG_CAT_CR "CR"
#define DSLOG_CAT_SG "SG"


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Opens connection to logger. Needs to be once per deepstream application execution, prior to use of the logger.
 */ 
void nvds_log_open();


/**
 * Called when application no longer needs logging capability 
 */
void nvds_log_close();

// Category is user (application) defined; priority is based on syslog levels
//     data is message to be logged
/**
 * Logs a message to locatioon as defined based on setup script
 *
 * @param[in] category User defined string to defined log category within application
 * @param[in] priority severity of log based on syslog levels (refer to README for more info)
 * @param[in] data Message to be logger formatted as C string as with printf 
 * @param[in] ... arguments corresponding to format 
 */
void nvds_log(const char *category, int priority, const char *data, ...);
#ifdef __cplusplus
}
#endif

#endif
