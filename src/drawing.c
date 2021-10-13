
#include <math.h>
#include "drawing.h"

void draw_line (GromitData *data,
		GdkDevice *dev,
		gint x1, gint y1,
		gint x2, gint y2)
{
  GdkRectangle rect;
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);

  rect.x = MIN (x1,x2) - data->maxwidth / 2;
  rect.y = MIN (y1,y2) - data->maxwidth / 2;
  rect.width = ABS (x1-x2) + data->maxwidth;
  rect.height = ABS (y1-y2) + data->maxwidth;

  if(data->debug)
    g_printerr("DEBUG: draw line from %d %d to %d %d\n", x1, y1, x2, y2);

  if (devdata->cur_context->paint_ctx)
    {
      cairo_set_line_width(devdata->cur_context->paint_ctx, data->maxwidth);
      cairo_set_line_cap(devdata->cur_context->paint_ctx, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join(devdata->cur_context->paint_ctx, CAIRO_LINE_JOIN_ROUND);

      cairo_move_to(devdata->cur_context->paint_ctx, x1, y1);
      cairo_line_to(devdata->cur_context->paint_ctx, x2, y2);
      cairo_stroke(devdata->cur_context->paint_ctx);

      data->modified = 1;

      gdk_window_invalidate_rect(gtk_widget_get_window(data->win), &rect, 0);
    }

  data->painted = 1;
}

void draw_arrow_when_applicable(GdkDevice *device, GromitDeviceData *devdata, GromitData *data, GromitArrowPosition position)
{
  gfloat direction = 0;
  gint width = 0;
  GromitStrokeCoordinate *arrow_point;

  if(devdata->cur_context->arrowsize == 0)
    return;

  if(!(devdata->cur_context->arrowposition & position))
    return;

  if(devdata->cur_context)
    width = devdata->cur_context->arrowsize * devdata->cur_context->width / 2;

  if(!coord_list_get_arrow_param (data, device, width * 3, position, &width, &direction))
    return;

  switch (position)
  {
  case GROMIT_ARROW_AT_START:
    if(devdata->cur_context->start_arrow_painted &&
      devdata->cur_context->type == GROMIT_PEN)
      return;

    arrow_point = g_list_last(devdata->coordlist)->data;
    draw_arrow (data, device, arrow_point->x, arrow_point->y, width, direction);
    devdata->cur_context->start_arrow_painted = TRUE;

    break;
  case GROMIT_ARROW_AT_END:
    arrow_point = devdata->coordlist->data;
    draw_arrow (data, device, arrow_point->x, arrow_point->y, width, direction);

    break;
  case GROMIT_ARROW_AT_BOTH:
  case GROMIT_ARROW_AT_NONE:
  default:
    break;
  }
}

void draw_ellipse(GromitData *data,
		GdkDevice *dev,
		gint x1, gint y1,
		gint x2, gint y2)
{
  GdkRectangle rect;
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);

  rect.x = MIN (x1,x2) - data->maxwidth / 2;
  rect.y = MIN (y1,y2) - data->maxwidth / 2;
  rect.width = ABS (x1-x2) + data->maxwidth;
  rect.height = ABS (y1-y2) + data->maxwidth;

  if(data->debug)
    g_printerr("DEBUG: draw ellipse coord (%d,%d) %dx%d\n", rect.x, rect.y, rect.width, rect.height);

  if (devdata->cur_context->paint_ctx)
    {
      cairo_save(devdata->cur_context->paint_ctx);
      cairo_translate(devdata->cur_context->paint_ctx, rect.x + (rect.width / 2.0), rect.y + (rect.height / 2.0));
      cairo_scale(devdata->cur_context->paint_ctx, rect.width / 2.0, rect.height / 2.0);
      cairo_arc(devdata->cur_context->paint_ctx, 0.0, 0.0, 1.0, 0.0, M_PI * 2);
      cairo_restore(devdata->cur_context->paint_ctx);

      cairo_set_line_width(devdata->cur_context->paint_ctx, data->maxwidth);
      cairo_set_line_cap(devdata->cur_context->paint_ctx, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join(devdata->cur_context->paint_ctx, CAIRO_LINE_JOIN_ROUND);

      cairo_stroke(devdata->cur_context->paint_ctx);

      data->modified = 1;

      gdk_window_invalidate_rect(gtk_widget_get_window(data->win), &rect, 0);
    }

  data->painted = 1;
}

void draw_ellipse_during_motion (GdkEventMotion *ev, GromitDeviceData *devdata, GromitData *data)
{
  GromitStrokeCoordinate *start_point = g_list_last(devdata->coordlist)->data;

  gint x0 = start_point->x;
  gint y0 = start_point->y;
  gint w0 = start_point->width;

  copy_surface(data->backbuffer, data->motionbuffer);
  GdkRectangle rect = {0, 0, data->width, data->height};
  gdk_window_invalidate_rect(gtk_widget_get_window(data->win), &rect, 0);
  data->modified = 1;

  draw_ellipse (data, ev->device, x0, y0, ev->x, ev->y);

  coord_list_free(data, ev->device);
  coord_list_prepend (data, ev->device, x0, y0, w0);
}

void draw_arrow (GromitData *data,
		 GdkDevice *dev,
		 gint x1, gint y1,
		 gint width,
		 gfloat direction)
{
  GdkRectangle rect;
  GdkPoint arrowhead [4];

  /* get the data for this device */
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);

  width = width / 2;

  /* I doubt that calculating the boundary box more exact is very useful */
  rect.x = x1 - 4 * width - 1;
  rect.y = y1 - 4 * width - 1;
  rect.width = 8 * width + 2;
  rect.height = 8 * width + 2;

  arrowhead [0].x = x1 + 4 * width * cos (direction);
  arrowhead [0].y = y1 + 4 * width * sin (direction);

  arrowhead [1].x = x1 - 3 * width * cos (direction)
                       + 3 * width * sin (direction);
  arrowhead [1].y = y1 - 3 * width * cos (direction)
                       - 3 * width * sin (direction);

  arrowhead [2].x = x1 - 2 * width * cos (direction);
  arrowhead [2].y = y1 - 2 * width * sin (direction);

  arrowhead [3].x = x1 - 3 * width * cos (direction)
                       - 3 * width * sin (direction);
  arrowhead [3].y = y1 + 3 * width * cos (direction)
                       - 3 * width * sin (direction);

  if (devdata->cur_context->paint_ctx)
    {
      cairo_set_line_width(devdata->cur_context->paint_ctx, 1);
      cairo_set_line_cap(devdata->cur_context->paint_ctx, CAIRO_LINE_CAP_ROUND);
      cairo_set_line_join(devdata->cur_context->paint_ctx, CAIRO_LINE_JOIN_ROUND);

      cairo_move_to(devdata->cur_context->paint_ctx, arrowhead[0].x, arrowhead[0].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[1].x, arrowhead[1].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[2].x, arrowhead[2].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[3].x, arrowhead[3].y);
      cairo_fill(devdata->cur_context->paint_ctx);

      gdk_cairo_set_source_rgba(devdata->cur_context->paint_ctx, data->black);

      cairo_move_to(devdata->cur_context->paint_ctx, arrowhead[0].x, arrowhead[0].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[1].x, arrowhead[1].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[2].x, arrowhead[2].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[3].x, arrowhead[3].y);
      cairo_line_to(devdata->cur_context->paint_ctx, arrowhead[0].x, arrowhead[0].y);
      cairo_stroke(devdata->cur_context->paint_ctx);

      gdk_cairo_set_source_rgba(devdata->cur_context->paint_ctx, devdata->cur_context->paint_color);

      data->modified = 1;

      gdk_window_invalidate_rect(gtk_widget_get_window(data->win), &rect, 0);
    }

  data->painted = 1;
}


void coord_list_prepend (GromitData *data,
			 GdkDevice* dev,
			 gint x,
			 gint y,
			 gint width)
{
  /* get the data for this device */
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);

  GromitStrokeCoordinate *point;

  point = g_malloc (sizeof (GromitStrokeCoordinate));
  point->x = x;
  point->y = y;
  point->width = width;

  devdata->coordlist = g_list_prepend (devdata->coordlist, point);
}


void coord_list_free (GromitData *data,
		      GdkDevice* dev)
{
  /* get the data for this device */
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);

  GList *ptr;
  ptr = devdata->coordlist;

  while (ptr)
    {
      g_free (ptr->data);
      ptr = ptr->next;
    }

  g_list_free (devdata->coordlist);

  devdata->coordlist = NULL;
}

void cleanup_context(GromitPaintContext *context)
{
  context->start_arrow_painted = FALSE;
}

gboolean coord_list_get_arrow_param (GromitData *data,
				     GdkDevice           *dev,
				     gint                search_radius,
             GromitArrowPosition position,
				     gint                *ret_width,
				     gfloat              *ret_direction)
{
  gint x0, y0, r2, dist;
  gboolean success = FALSE;
  GromitStrokeCoordinate  *cur_point, *valid_point;
  /* get the data for this device */
  GromitDeviceData *devdata = g_hash_table_lookup(data->devdatatable, dev);
  GList *ptr;
  gfloat width;

  valid_point = NULL;

  if(position == GROMIT_ARROW_AT_END)
    ptr = devdata->coordlist;
  else if(position == GROMIT_ARROW_AT_START)
    ptr = g_list_last(devdata->coordlist);

  if (ptr)
    {
      cur_point = ptr->data;
      x0 = cur_point->x;
      y0 = cur_point->y;
      r2 = search_radius * search_radius;
      dist = 0;

      while (ptr && dist < r2)
        {
          if(position == GROMIT_ARROW_AT_END)
            ptr = ptr->next;
          else if(position == GROMIT_ARROW_AT_START)
            ptr = ptr->prev;

          if (ptr)
            {
              cur_point = ptr->data;
              dist = (cur_point->x - x0) * (cur_point->x - x0) +
                     (cur_point->y - y0) * (cur_point->y - y0);
              width = cur_point->width * devdata->cur_context->arrowsize;
              if (width * 2 <= dist &&
                  (!valid_point || valid_point->width <= cur_point->width))
                  valid_point = cur_point;
            }
        }

      if (valid_point)
        {
          *ret_width = MAX (valid_point->width * devdata->cur_context->arrowsize,
                            2);
          *ret_direction = atan2 (y0 - valid_point->y, x0 - valid_point->x);
           success = TRUE;
        }
    }

  return success;
}

void draw_straight_line_during_motion (GdkEventMotion *ev, GromitDeviceData *devdata, GromitData *data)
{
  GromitStrokeCoordinate *start_point = g_list_last(devdata->coordlist)->data;

  gint x0 = start_point->x;
  gint y0 = start_point->y;
  gint w0 = start_point->width;

  //Restore initial surface (before start drawing)
  copy_surface(data->backbuffer, data->motionbuffer);
  GdkRectangle rect = {0, 0, data->width, data->height};
  gdk_window_invalidate_rect(gtk_widget_get_window(data->win), &rect, 0);
  data->modified = 1;

  //Recreate the line every motion
  draw_line (data, ev->device, x0, y0, ev->x, ev->y);

  //Straight lines only needs 2 coord points
  coord_list_free(data, ev->device);
  coord_list_prepend (data, ev->device, x0, y0, w0);
}
