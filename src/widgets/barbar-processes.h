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

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * BarBarProcessOrder:
 * @BARBAR_ORDER_MEM: Sort processes by mem usage
 * @BARBAR_ORDER_CPU: Sort processes by cpu usage
 *
 * How a processes list should be sorted
 */
typedef enum {
  BARBAR_ORDER_MEM,
  BARBAR_ORDER_CPU,
  BARBAR_ORDER_IO,
} BarBarProcessOrder;

GType g_barbar_procces_order_get_type(void);

#define BARBAR_TYPE_PROCESS_ORDER (g_barbar_procces_order_get_type())

#define BARBAR_TYPE_CPU_PROCESSES (g_barbar_cpu_processes_get_type())

G_DECLARE_FINAL_TYPE(BarBarCpuProcesses, g_barbar_cpu_processes, BARBAR,
                     CPU_PROCESSES, GtkWidget)

GtkWidget *g_barbar_cpu_processes_new(void);

void g_barbar_cpu_processes_set_number(BarBarCpuProcesses *cpu, guint number);

void g_barbar_cpu_processes_set_interval(BarBarCpuProcesses *cpu,
                                         guint interval);

void g_barbar_cpu_processes_set_order(BarBarCpuProcesses *cpu,
                                      BarBarProcessOrder order);

void g_barbar_cpu_processes_set_seperate_cpu(BarBarCpuProcesses *cpu,
                                             gboolean seperate);
G_END_DECLS
