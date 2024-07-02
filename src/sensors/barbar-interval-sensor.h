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

#pragma once

#include "barbar-sensor.h"

G_BEGIN_DECLS

#define BARBAR_TYPE_INTERVAL_SENSOR (g_barbar_sensor_get_type())

G_DECLARE_DERIVABLE_TYPE(BarBarIntervalSensor, g_barbar_interval_sensor, BARBAR,
                         INTERVAL_SENSOR, BarBarSensor)

struct _BarBarIntervalSensorClass {
  GObjectClass parent_class;

  gboolean (*tick)(BarBarIntervalSensor *self);
};

void g_barbar_interval_sensor_set_interval(BarBarIntervalSensor *self,
                                           uint interval);

G_END_DECLS
