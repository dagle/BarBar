/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Per Odlund
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _BARBAR_CALENDAR_H_
#define _BARBAR_CALENDAR_H_

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* #define BARBAR_TYPE_CLOCK (g_barbar_clock_get_type()) */

#define BARBAR_TYPE_CALENDAR (g_barbar_calendar_get_type())
#define BARBAR_CALENDAR(obj)                                                   \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), BARBAR_TYPE_CALENDAR, BarBarCalendar))
#define BABBAR_IS_CALENDAR(obj)                                                \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), BARBAR_TYPE_CALENDAR))

typedef struct _BarBarCalendar BarBarCalendar;
// TODO: This shouldn't have GObject as parent
/* G_DECLARE_FINAL_TYPE(BarBarCalendar, g_barbar_clock, BARBAR, CALANDAR, */
/*                      GtkWidget) */

GType barbar_calendar_get_type(void) G_GNUC_CONST;

GtkWidget *barbar_calendar_new(void);
/* void g_barbar_clock_start(BarBarCalendar *clock); */

G_END_DECLS

#endif /* _BARBAR_CALENDAR_H_ */
