#ifndef DRAWING_H
#define DRAWING_H

/*
  Functions that manipulate the surfaces pixel-wise.
  Does not include functions that treat a surface like a buffer, like undo/redo etc.
*/

#include "main.h"

typedef struct
{
  gint x;
  gint y;
  gint width;
} GromitStrokeCoordinate;


void draw_line (GromitData *data, GdkDevice *dev, gint x1, gint y1, gint x2, gint y2);
void draw_arrow (GromitData *data, GdkDevice *dev, gint x1, gint y1, gint width, gfloat direction);
void draw_arrow_when_applicable(GdkDevice *device, GromitDeviceData *devdata, GromitData *data, GromitArrowPosition position);
void draw_ellipse(GromitData *data, GdkDevice *dev, gint x1, gint y1, gint x2, gint y2);
void draw_rectangle(GromitData *data, GdkDevice *dev, gint x1, gint y1, gint x2, gint y2);
void draw_shape_during_motion (
  GdkEventMotion *ev,
  GromitDeviceData *devdata,
  GromitData *data,
  void (*draw_shape) (GromitData*, GdkDevice*, gint, gint, gint, gint));
void cleanup_context(GromitPaintContext *context);
gboolean coord_list_get_arrow_param (GromitData *data,
					    GdkDevice  					*dev,
					    gint        				search_radius,
							GromitArrowPosition arrowposition,
					    gint       					*ret_width,
					    gfloat     					*ret_direction);
void coord_list_prepend (GromitData *data, GdkDevice* dev, gint x, gint y, gint width);
void coord_list_free (GromitData *data, GdkDevice* dev);


#endif
