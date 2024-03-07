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

#ifndef _BARBAR_DBUSMENU_H_
#define _BARBAR_DBUSMENU_H_

#include "status-notifier.h"
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_DBUS_MENU (g_barbar_dbus_menu_get_type())

G_DECLARE_FINAL_TYPE(BarBarDBusMenu, g_barbar_dbus_menu, BARBAR, DBUS_MENU,
                     GtkWidget)

BarBarDBusMenu *g_barbar_dbus_menu_new(const gchar *bus_name,
                                       const gchar *path);

gint g_handler(BarBarDBusMenu *menu);
void g_barbar_dbus_menu_event(BarBarDBusMenu *menu, int id);

G_END_DECLS

#endif /* _BARBAR_DBUSMENU_H_ */
