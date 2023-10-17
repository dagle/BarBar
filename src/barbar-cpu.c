#include "barbar-cpu.h"
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <math.h>
#include <stdio.h>

struct _BarBarCpu {
  GObject parent;

  // TODO:This should be in parent
  char *label;

};

enum {
  PROP_0,

  PROP_TZ,

  NUM_PROPERTIES,
};

G_DEFINE_TYPE(BarBarCpu, g_barbar_cpu, G_TYPE_OBJECT)

static GParamSpec *cpu_props[NUM_PROPERTIES] = {
    NULL,
};

static void g_barbar_cpu_set_property(GObject *object, guint property_id,
                                  const GValue *value, GParamSpec *pspec) {
}

static void g_barbar_cpu_get_property(GObject *object, guint property_id,
                                  GValue *value, GParamSpec *pspec) {
}

static void g_barbar_cpu_class_init(BarBarCpuClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);

  gobject_class->set_property = g_barbar_cpu_set_property;
  gobject_class->get_property = g_barbar_cpu_get_property;
  cpu_props[PROP_TZ] = g_param_spec_string(
      "path", NULL, NULL, "/", G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES, cpu_props);
}

static void g_barbar_cpu_init(BarBarCpu *self) {
}

void g_barbar_cpu_update(BarBarCpu *self) {
	unsigned long frequency;
	double total, user, nice, sys, idle;
	double b_total, b_user, b_nice, b_sys, b_idle;
	double s_total, s_user, s_nice, s_sys, s_idle;
	char separator [BUFSIZ], buffer [BUFSIZ];
	int ncpu, i;
	glibtop_cpu cpu;

	glibtop_init();

	glibtop_get_cpu (&cpu);

	ncpu = glibtop_global_server->ncpu ? glibtop_global_server->ncpu : 1;

	frequency = (unsigned long) cpu.frequency;

	total = ((unsigned long) cpu.total) ? ((double) cpu.total) : 1.0;
	user  = ((unsigned long) cpu.user)  ? ((double) cpu.user)  : 1.0;
	nice  = ((unsigned long) cpu.nice)  ? ((double) cpu.nice)  : 1.0;
	sys   = ((unsigned long) cpu.sys)   ? ((double) cpu.sys)   : 1.0;
	idle  = ((unsigned long) cpu.idle)  ? ((double) cpu.idle)  : 1.0;

	s_total = s_user = s_nice = s_sys = s_idle = 0.0;

	b_total = total / ncpu;
	b_user  = user  / ncpu;
	b_nice  = nice  / ncpu;
	b_sys   = sys   / ncpu;
	b_idle  = idle  / ncpu;

	memset (separator, '-', 91);
	separator [92] = '\0';

	sprintf (buffer, "Ticks (%ld per second):", frequency);

	printf ("\n\n%-26s %12s %12s %12s %12s %12s\n%s\n", buffer,
		"Total", "User", "Nice", "Sys", "Idle", separator);

	printf ("CPU          (0x%08lx): %12.0f %12.0f %12.0f %12.0f %12.0f\n\n",
		(unsigned long) cpu.flags, total, user, nice, sys, idle);

	for (i = 0; i < glibtop_global_server->ncpu; i++) {
		printf ("CPU %3d      (0x%08lx): %12lu %12lu %12lu %12lu %12lu\n", i,
			(unsigned long) cpu.flags,
			(unsigned long) cpu.xcpu_total [i],
			(unsigned long) cpu.xcpu_user  [i],
			(unsigned long) cpu.xcpu_nice  [i],
			(unsigned long) cpu.xcpu_sys   [i],
			(unsigned long) cpu.xcpu_idle  [i]);

		s_total += fabs (((double) cpu.xcpu_total [i]) - b_total);
		s_user  += fabs (((double) cpu.xcpu_user  [i]) - b_user);
		s_nice  += fabs (((double) cpu.xcpu_nice  [i]) - b_nice);
		s_sys   += fabs (((double) cpu.xcpu_sys   [i]) - b_sys);
		s_idle  += fabs (((double) cpu.xcpu_idle  [i]) - b_idle);
	}

	printf ("%s\n\n\n", separator);

	printf ("%-26s %12s %12s %12s %12s %12s\n%s\n", "Percent:",
		"Total (%)", "User (%)", "Nice (%)", "Sys (%)",
		"Idle (%)", separator);

	printf ("CPU          (0x%08lx): %12.3f %12.3f %12.3f %12.3f %12.3f\n\n",
		(unsigned long) cpu.flags, (double) total * 100.0 / total,
		(double) user  * 100.0 / total,
		(double) nice  * 100.0 / total,
		(double) sys   * 100.0 / total,
		(double) idle  * 100.0 / total);

	for (i = 0; i < glibtop_global_server->ncpu; i++) {
		double p_total, p_user, p_nice, p_sys, p_idle;

		p_total = ((double) cpu.xcpu_total [i]) * 100.0 / total;
		p_user  = ((double) cpu.xcpu_user  [i]) * 100.0 / user;
		p_nice  = ((double) cpu.xcpu_nice  [i]) * 100.0 / nice;
		p_sys   = ((double) cpu.xcpu_sys   [i]) * 100.0 / sys;
		p_idle  = ((double) cpu.xcpu_idle  [i]) * 100.0 / idle;

		printf ("CPU %3d      (0x%08lx): %12.3f %12.3f %12.3f %12.3f %12.3f\n",
			i, (unsigned long) cpu.flags, p_total, p_user, p_nice,
			p_sys, p_idle);
	}

	printf ("%s\n%-26s %12.3f %12.3f %12.3f %12.3f %12.3f\n\n", separator,
		"Spin:", s_total * 100.0 / total, s_user * 100.0 / user,
		s_nice * 100.0 / nice, s_sys * 100.0 / sys, s_idle * 100.0 / idle);
}
