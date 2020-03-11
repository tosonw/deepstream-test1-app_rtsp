/*
 * Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * <b>NVIDIA Multimedia Utilities: On-Screen Display Manager</b>
 *
 * This file defines the NvOSD library to be used to draw rectangles and text over the frame
 * for given parameters.
 */

/**
 * @defgroup ee_nvosd_group On-Screen Display Manager
 * Defines the NvOSD library to be used to draw rectangles and text over the frame
 * for given parameters.
 * @ingroup common_utility_group
 * @{
 */

#ifndef __NVLL_OSD_API_DEFS__
#define __NVLL_OSD_API_DEFS__

#include "nvll_osd_struct.h"
#include "nvbufsurface.h"

#define NVOSD_PRINT_E(f_, ...) \
  printf("libnvosd (%d):(ERROR) : " f_, __LINE__, ##__VA_ARGS__)

#define MAX_IN_BUF 16
#define MAX_BORDER_WIDTH 32

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct _NvOSD_FrameTextParams
{
  /** Holds pointer to the buffer containing frame. */
  NvBufSurfaceParams *buf_ptr;
  /** Holds OSD Mode to be used for processing. */
  NvOSD_Mode mode;
  /** Holds number of strings. */
  int num_strings;
  /** Holds text params of string. */
  NvOSD_TextParams *text_params_list;
} NvOSD_FrameTextParams;

typedef struct _NvOSD_FrameRectParams
{
  /** Holds pointer to the buffer containing frame. */
  NvBufSurfaceParams *buf_ptr;
  /** Holds OSD Mode to be used for processing. */
  NvOSD_Mode mode;
  /** Holds number of Rectangles. */
  int num_rects;
  /** Holds params of Rectangles. */
  NvOSD_RectParams *rect_params_list;
} NvOSD_FrameRectParams;

typedef struct _NvOSD_FrameLineParams
{
  /** Holds pointer to the buffer containing frame. */
  NvBufSurfaceParams *buf_ptr;
  /** Holds OSD Mode to be used for processing. */
  NvOSD_Mode mode;
  /** Holds number of Lines. */
  int num_lines;
  /** Holds params of Lines. */
  NvOSD_LineParams *line_params_list;
} NvOSD_FrameLineParams;

/**
 * Creates an NvOSD context.
 *
 * @returns A pointer to NvOSD context, NULL in case of failure.
 */
void *nvll_osd_create_context(void);

/**
 * Destroys an NvOSD context.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 */

void nvll_osd_destroy_context(void *nvosd_ctx);

/**
 * Sets clock parameters for the given context.
 *
 * The clock is overlayed when nvll_osd_put_text() is called.
 * If no other text is to be overlayed, nvll_osd_put_text must be called with
 * @a num_strings as 0 and @a text_params_list as NULL.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 * @param[in] clk_params A pointer to NvOSD_TextParams structure for the clock
 *            to be overlayed; NULL to disable the clock.
 */
void nvll_osd_set_clock_params(void *nvosd_ctx, NvOSD_TextParams *clk_params);


/**
 * Overlays clock and given text at given location on a buffer.
 *
 * To overlay the clock, you must set clock params using
 * nvll_osd_set_clock_params(). Additionally,
 * you must ensure that the length of @a text_params_list is at least
 * @a num_strings.
 *
 * @note Currently only MODE_CPU is supported. Specifying other modes wil have
 * no effect.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 * @param[in] frame_text_params A pointer to the FrameTextParams struct
 *            containing rectangles information to be overlayed
 *
 * @returns 0 for success, -1 for failure.
 */
int nvll_osd_put_text(void *nvosd_ctx, NvOSD_FrameTextParams *frame_text_params);

/**
 * Overlays boxes at given location on a buffer.
 *
 * Boxes can be configured with:
 * a. Only border
 *    To draw boxes with only border, you must set @a border_width and set
 *    @a has_bg_color to 0 for the given box.
 * b. Border and background color
 *    To draw boxes with border and background color, you must set @a
 *    border_width and set @a has_bg_color to 1, and specify background color
 *    parameters for the given box.
 * c. Solid fill acting as mask region
 *    To draw boxes with solid fill acting as mask region, you must set @a
 *    border_width to 0 and @a has_bg_color to 1 for the given box.
 *
 *
 * You must ensure that the length of @a rect_params_list is at least
 * @a num_rects.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 * @param[in] frame_rect_params A pointer to the FrameRectParams struct
 *            containing rectangles information to be overlayed.
 *
 * @returns 0 for success, -1 for failure.
 */
int nvll_osd_draw_rectangles(void *nvosd_ctx, NvOSD_FrameRectParams *frame_rect_params);

/**
 * Overlays lines on the buffer.
 *
 * The length of @a line_params_list must be equal to @a num_lines.
 * The client is responsible for allocating this array.
 *
 * @note Currently only MODE_CPU is supported. Specifying other modes will have
 * no effect.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 * @param[in] frame_line_params A pointer to the FrameLineParams struct
 *            containing line information to be overlayed.
 *
 * @returns 0 for success, -1 for failure.
 */
int nvll_osd_draw_lines(void *nvosd_ctx, NvOSD_FrameLineParams *frame_line_params);

/**
 * Sets the resolution of the frames on which the NvOSDContext will operate.
 *
 * @param[in] nvosd_ctx A pointer to NvOSD context.
 * @param[in] width Width of the input frames.
 * @param[in] height Height of the input frames.
 *
 * @returns 0 for success, -1 for failure.
 */
void *nvll_osd_set_params (void *nvosd_ctx, int width, int height);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
