#include "barbar-network.h"
#include "glibconfig.h"
#include <glibtop/netload.h>
#include <glibtop/parameter.h>

#include <glibtop/close.h>
#include <glibtop/open.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <libintl.h>
#include <locale.h>

#include <stdint.h>
#include <stdio.h>

struct _BarBarNetwork {
  BarBarSensor parent_instance;

  char *interface;
  int family;

  guint interval;

  guint source_id;

  guint64 bytes_in;
  guint64 bytes_out;

  guint64 up_speed;
  guint64 down_speed;
};

enum {
  PROP_0,

  PROP_INTERVAL,
  PROP_INTERFACE,

  PROP_UP_SPEED,
  PROP_DOWN_SPEED,

  NUM_PROPERTIES,
};

enum {
  TICK,
  NUM_SIGNALS,
};

#ifndef PROFILE_COUNT
#define PROFILE_COUNT 1
#endif

// update every 10 sec
#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarNetwork, g_barbar_network, BARBAR_TYPE_SENSOR)

static GParamSpec *network_props[NUM_PROPERTIES] = {
    NULL,
};

static guint network_signals[NUM_SIGNALS];

static void g_barbar_network_set_interface(BarBarNetwork *network,
                                           const char *interface) {
  g_return_if_fail(BARBAR_IS_NETWORK(network));

  g_free(network->interface);
  network->interface = g_strdup(interface);

  g_object_notify_by_pspec(G_OBJECT(network), network_props[PROP_INTERVAL]);
}

static void g_barbar_network_set_interval(BarBarNetwork *network,
                                          guint interval) {
  g_return_if_fail(BARBAR_IS_NETWORK(network));

  network->interval = interval;
  if (network->source_id > 0) {
  }

  g_object_notify_by_pspec(G_OBJECT(network), network_props[PROP_INTERVAL]);
}
static void g_barbar_network_set_property(GObject *object, guint property_id,
                                          const GValue *value,
                                          GParamSpec *pspec) {

  BarBarNetwork *network = BARBAR_NETWORK(object);
  switch (property_id) {
  case PROP_INTERFACE:
    g_barbar_network_set_interface(network, g_value_get_string(value));
    break;
  case PROP_INTERVAL:
    g_barbar_network_set_interval(network, g_value_get_uint(value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_network_get_property(GObject *object, guint property_id,
                                          GValue *value, GParamSpec *pspec) {
  BarBarNetwork *network = BARBAR_NETWORK(object);

  switch (property_id) {
  case PROP_INTERFACE:
    g_value_set_string(value, network->interface);
    break;
  case PROP_INTERVAL:
    g_value_set_uint(value, network->interval);
    break;
  case PROP_UP_SPEED:
    g_value_set_uint64(value, network->up_speed);
    break;
  case PROP_DOWN_SPEED:
    g_value_set_uint64(value, network->down_speed);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_network_constructed(GObject *obj);

static void g_barbar_network_class_init(BarBarNetworkClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  BarBarSensorClass *sensor_class = BARBAR_SENSOR_CLASS(class);

  sensor_class->start = g_barbar_network_start;

  gobject_class->set_property = g_barbar_network_set_property;
  gobject_class->get_property = g_barbar_network_get_property;
  gobject_class->constructed = g_barbar_network_constructed;

  /**
   * BarBarNetwork:interval:
   *
   * How often network should be pulled for info
   */
  network_props[PROP_INTERVAL] = g_param_spec_uint(
      "interval", "Interval", "Interval in milli seconds", 0, G_MAXUINT32,
      DEFAULT_INTERVAL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarNetwork:interface:
   *
   * The name of the interface
   */
  network_props[PROP_INTERFACE] = g_param_spec_string(
      "interface", "Interface", "Interface to measure speed on", NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT);

  /**
   * BarBarNetwork:up-speed:
   *
   * The upload speed of the device
   */
  network_props[PROP_UP_SPEED] = g_param_spec_uint64(
      "up-speed", NULL, NULL, 0, UINT64_MAX, 0, G_PARAM_READABLE);
  /**
   * BarBarNetwork:down-speed:
   *
   * The download speed of the device
   */
  network_props[PROP_DOWN_SPEED] = g_param_spec_uint64(
      "down-speed", NULL, NULL, 0, UINT64_MAX, 0, G_PARAM_READABLE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    network_props);

  /**
   * BarBarNetwork::tick:
   * @sensor: This sensor
   *
   * Emit that the network has ticked. This means that we want to refetch
   * the network.
   */
  network_signals[TICK] =
      g_signal_new("tick",                                 /* signal_name */
                   BARBAR_TYPE_NETWORK,                    /* itype */
                   G_SIGNAL_RUN_FIRST | G_SIGNAL_DETAILED, /* signal_flags */
                   0,                                      /* class_offset */
                   NULL,                                   /* accumulator */
                   NULL,                                   /* accu_data */
                   NULL,                                   /* c_marshaller */
                   G_TYPE_NONE,                            /* return_type */
                   0                                       /* n_params */
      );
}

static void g_barbar_network_init(BarBarNetwork *self) {
  self->interval = DEFAULT_INTERVAL;
  self->bytes_out = 0;
  self->bytes_in = 0;
}

static void g_barbar_network_constructed(GObject *obj) {
  BarBarNetwork *self = BARBAR_NETWORK(obj);
  G_OBJECT_CLASS(g_barbar_network_parent_class)->constructed(obj);
}

static char *hwaddress_format_for_display(glibtop_netload *buf) {
  unsigned i;
  GString *repr = g_string_new("");
  char *str;

  for (i = 0; i < sizeof buf->hwaddress; ++i)
    g_string_append_printf(repr, "%02X:", ((unsigned char *)buf->hwaddress)[i]);

  repr->str[repr->len - 1] = ' ';
  str = g_string_free(repr, FALSE);
  g_strstrip(str);
  return str;
}

static gboolean g_barbar_network_update(gpointer data) {
  BarBarNetwork *net = BARBAR_NETWORK(data);

  glibtop_netload netload;
  struct in_addr addr, subnet;
  char address_string[INET_ADDRSTRLEN], subnet_string[INET_ADDRSTRLEN];
  char address6_string[INET6_ADDRSTRLEN], prefix6_string[INET6_ADDRSTRLEN];

  if (net->interface == NULL) {
    return G_SOURCE_REMOVE;
  }

  glibtop_get_netload(&netload, net->interface);

  addr.s_addr = netload.address;
  subnet.s_addr = netload.subnet;

  inet_ntop(AF_INET, &addr, address_string, INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &subnet, subnet_string, INET_ADDRSTRLEN);
  inet_ntop(AF_INET6, netload.address6, address6_string, INET6_ADDRSTRLEN);
  inet_ntop(AF_INET6, netload.prefix6, prefix6_string, INET6_ADDRSTRLEN);

  if (!net->bytes_in) {
    net->bytes_in = netload.bytes_in;
  }
  if (!net->bytes_in) {
    net->bytes_out = netload.bytes_out;
  }

  guint tick = net->interval / 1000;

  net->down_speed = (netload.bytes_in - net->bytes_in) / tick;
  net->up_speed = (netload.bytes_out - net->bytes_out) / tick;

  net->bytes_in = netload.bytes_in;
  net->bytes_out = netload.bytes_out;

  g_object_notify_by_pspec(G_OBJECT(net), network_props[PROP_UP_SPEED]);
  g_object_notify_by_pspec(G_OBJECT(net), network_props[PROP_DOWN_SPEED]);

  g_signal_emit(net, network_signals[TICK], 0);

  return G_SOURCE_CONTINUE;
}

void g_barbar_network_start(BarBarSensor *sensor) {
  BarBarNetwork *net = BARBAR_NETWORK(sensor);
  if (net->source_id > 0) {
    g_source_remove(net->source_id);
  }
  g_barbar_network_update(net);
  net->source_id =
      g_timeout_add_full(0, net->interval, g_barbar_network_update, net, NULL);
}
