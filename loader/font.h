#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

/**
 * @brief Font raster backend for GFA bridge (NexusFont.java).
 */

int gfa_font_init(const char *path); // 1 si cargo bien (idempotente)
int gfa_font_ready(void);

int gfa_font_ascent(float px);
int gfa_font_descent(float px);

/**
 * @brief Font raster backend for GFA bridge (NexusFont.java).
 */
float gfa_font_advance(float px, uint32_t cp);
float gfa_font_text_width(float px, const uint32_t *cps, int n);

/**
 * @brief Font raster backend for GFA bridge (NexusFont.java).
 */
int gfa_font_break_text(float px, const uint32_t *cps, int n, float max_width);

/**< @brief Font raster backend for GFA bridge (NexusFont.java). */
float gfa_font_draw_line(float px, const uint32_t *cps, int n,
                         uint32_t *buf, int bw, int bh,
                         float pen_x, float baseline_y, uint32_t color);

#endif
