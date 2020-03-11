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

#ifndef __NVLL_OSD_STRUCT_DEFS__
#define __NVLL_OSD_STRUCT_DEFS__

#ifdef __cplusplus
extern "C"
{
#endif

#define NVOSD_MAX_NUM_RECTS 128
#define MAX_BG_CLR 20

/**
 * List modes used to overlay boxes and text
 */
typedef enum {
    MODE_CPU, /**< Selects CPU for OSD processing.
                Works with RGBA data only */
    MODE_GPU, /**< Selects GPU for OSD processing.
                Yet to be implemented */
    MODE_HW   /**< Selects NV HW engine for rectangle draw and mask.
                   This mode works with both YUV and RGB data.
                   It does not consider alpha parameter.
                   Not applicable for drawing text. */
} NvOSD_Mode;

/**
 * Holds the color parameters of the box or text to be overlayed.
 */
typedef struct _NvOSD_ColorParams {
  double red;                 /**< Holds red component of color.
                                   Value must be in the range 0-1. */

  double green;               /**< Holds green component of color.
                                   Value must be in the range 0-1.*/

  double blue;                /**< Holds blue component of color.
                                   Value must be in the range 0-1.*/

  double alpha;               /**< Holds alpha component of color.
                                   Value must be in the range 0-1.*/
} NvOSD_ColorParams;

/**
 * Holds the font parameters of the text to be overlayed.
 */
typedef struct _NvOSD_FontParams {
  char * font_name;         /**< Holds pointer to the string containing
                                      font name. The list of supported fonts
                                      can be obtained by running fc-list
                                      command */

//  char font_name[64];               /**< Holds pointer to the string containing
//                                      font name. */

  unsigned int font_size;         /**< Holds size of the font. */

  NvOSD_ColorParams font_color;   /**< Holds font color. */
} NvOSD_FontParams;


/**
 * Holds the text parameters of the text to be overlayed.
 */

typedef struct _NvOSD_TextParams {
  char * display_text; /**< Holds the text to be overlayed. */

  unsigned int x_offset; /**< Holds horizontal offset w.r.t top left pixel of
                             the frame. */
  unsigned int y_offset; /**< Holds vertical offset w.r.t top left pixel of
                             the frame. */

  NvOSD_FontParams font_params;/**< font_params. */

  int set_bg_clr; /**< Boolean to indicate text has background color. */

  NvOSD_ColorParams text_bg_clr; /**< Background color for text. */

} NvOSD_TextParams;

typedef struct _NvOSD_Color_info {
    int id;
    NvOSD_ColorParams color;
}NvOSD_Color_info;

/**
 * Holds the box parameters of the box to be overlayed.
 */
typedef struct _NvOSD_RectParams {
  unsigned int left;   /**< Holds left coordinate of the box in pixels. */

  unsigned int top;    /**< Holds top coordinate of the box in pixels. */

  unsigned int width;  /**< Holds width of the box in pixels. */

  unsigned int height; /**< Holds height of the box in pixels. */

  unsigned int border_width; /**< Holds border_width of the box in pixels. */

  NvOSD_ColorParams border_color; /**< Holds color params of the border
                                      of the box. */

  unsigned int has_bg_color;  /**< Holds boolean value indicating whether box
                                    has background color. */

  unsigned int reserved; /**< Reserved field for future usage.
                             For internal purpose only */

  NvOSD_ColorParams bg_color; /**< Holds background color of the box. */

  int has_color_info;
  int color_id;
} NvOSD_RectParams;

/**
 * Holds the box parameters of the line to be overlayed.
 */
typedef struct _NvOSD_LineParams {
  unsigned int x1;   /**< Holds left coordinate of the box in pixels. */

  unsigned int y1;    /**< Holds top coordinate of the box in pixels. */

  unsigned int x2;  /**< Holds width of the box in pixels. */

  unsigned int y2; /**< Holds height of the box in pixels. */

  unsigned int line_width; /**< Holds border_width of the box in pixels. */

  NvOSD_ColorParams line_color; /**< Holds color params of the border
                                        of the box. */
} NvOSD_LineParams;

#ifdef __cplusplus
}
#endif
/** @} */
#endif
