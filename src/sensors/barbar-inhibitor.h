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

#ifndef _BARBAR_INHIBITOR_H_
#define _BARBAR_INHIBITOR_H_

#include "sensors/barbar-sensorcontext.h"
#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

#define BARBAR_TYPE_INHIBITOR (g_barbar_inhibitor_get_type())

G_DECLARE_FINAL_TYPE(BarBarInhibitor, g_barbar_inhibitor, BARBAR, INHIBITOR,
                     GObject)

/* void g_barbar_inhibitor_start(BarBarInhibitor *inhibitor); */

G_END_DECLS

#endif /* _BARBAR_INHIBITOR_H_ */