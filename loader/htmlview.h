#ifndef HTMLVIEW_H
#define HTMLVIEW_H

#include <stdint.h>

/**
 * @brief Load app0:html/<name>.
 */
typedef struct htmlview htmlview;

/**
 * @brief Load app0:html/<name>.
 */
htmlview *htmlview_load(const char *name, float panel_w_px, float font_px);

/**
 * @brief Load app0:html/<name>.
 */
htmlview *htmlview_make_label(const char *text, float font_px, uint32_t color);

/**
 * @brief Load app0:html/<name>.
 */
void htmlview_draw(htmlview *v, float x, float y, float w, float h);

/**
 * @brief Load app0:html/<name>.
 */
void htmlview_scroll(htmlview *v, float delta_px, float viewport_h);

/**
 * @brief Load app0:html/<name>.
 */
void htmlview_native_size(htmlview *v, float *w, float *h);

#endif
