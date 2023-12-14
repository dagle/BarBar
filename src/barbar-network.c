#include "barbar-network.h"
#include <glibtop/netload.h>
#include <glibtop/parameter.h>

#include <glibtop/close.h>
#include <glibtop/open.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <libintl.h>
#include <locale.h>

#include <stdio.h>

struct _BarBarNetwork {
  GtkWidget parent_instance;

  GtkWidget *label;

  char *interface;
  int family;

  guint interval;

  guint source_id;

  guint64 bytes_in;
  guint64 bytes_out;
};

enum {
  PROP_0,

  PROP_INTERVAL,
  PROP_INTERFACE,

  NUM_PROPERTIES,
};

#ifndef PROFILE_COUNT
#define PROFILE_COUNT 1
#endif

// update every 10 sec
#define DEFAULT_INTERVAL 10000

G_DEFINE_TYPE(BarBarNetwork, g_barbar_network, GTK_TYPE_WIDGET)

static GParamSpec *network_props[NUM_PROPERTIES] = {
    NULL,
};

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
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
  }
}

static void g_barbar_network_class_init(BarBarNetworkClass *class) {
  GObjectClass *gobject_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  gobject_class->set_property = g_barbar_network_set_property;
  gobject_class->get_property = g_barbar_network_get_property;
  network_props[PROP_INTERVAL] =
      g_param_spec_uint("interval", "Interval", "Interval in milli seconds", 0,
                        G_MAXUINT32, DEFAULT_INTERVAL, G_PARAM_READWRITE);
  network_props[PROP_INTERFACE] = g_param_spec_string(
      "interface", "Interface", "Interface to measure speed on", NULL,
      G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, NUM_PROPERTIES,
                                    network_props);
  gtk_widget_class_set_layout_manager_type(widget_class, GTK_TYPE_BOX_LAYOUT);
}

void g_barbar_network_start(BarBarNetwork *net, gpointer data);

static void g_barbar_network_init(BarBarNetwork *self) {
  self->interval = DEFAULT_INTERVAL;
  self->label = gtk_label_new("");
  self->bytes_out = 0;
  self->bytes_in = 0;

  gtk_widget_set_parent(self->label, GTK_WIDGET(self));

  g_signal_connect(self, "map", G_CALLBACK(g_barbar_network_start), NULL);
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

  guint64 in_diff;
  guint64 out_diff;

  glibtop_netload netload;
  // unsigned method, count, port;
  // unsigned count;
  struct in_addr addr, subnet;
  char address_string[INET_ADDRSTRLEN], subnet_string[INET_ADDRSTRLEN];
  char address6_string[INET6_ADDRSTRLEN], prefix6_string[INET6_ADDRSTRLEN];
  // char *hwaddress_string;

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

  // hwaddress_string = hwaddress_format_for_display(&netload);

  if (!net->bytes_in) {
    net->bytes_in = netload.bytes_in;
    net->bytes_out = netload.bytes_out;
  }

  in_diff = netload.bytes_in - net->bytes_in;
  out_diff = netload.bytes_out - net->bytes_out;

  net->bytes_in = netload.bytes_in;
  net->bytes_out = netload.bytes_out;

  guint tick = net->interval / 1000;

  // printf("bytes in: %lu kb/s\n", in_diff / (1024 * (net->interval / 1000)));
  // printf("bytes out: %lu kb/s\n", out_diff / (1024 * (net->interval /
  // 1000)));

  gchar *str =
      g_strdup_printf("in: %lu kb/s | out: %lu kb/s", in_diff / (1024 * tick),
                      out_diff / (1024 * tick));
  gtk_label_set_label(GTK_LABEL(net->label), str);
  g_free(str);

  // printf("Network Load (0x%016llx):\n\n"
  //        "\tInterface Flags:\t0x%016llx\n"
  //        "\tAddress:\t\t0x%08x - %s\n"
  //        "\tSubnet:\t\t\t0x%08x - %s\n\n"
  //        "\tMTU:\t\t\t%d\n"
  //        "\tCollisions:\t\t%" G_GUINT64_FORMAT "\n\n"
  //        "\tPackets In:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tPackets Out:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tPackets Total:\t\t%" G_GUINT64_FORMAT "\n\n"
  //        "\tBytes In:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tBytes Out:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tBytes Total:\t\t%" G_GUINT64_FORMAT "\n\n"
  //        "\tErrors In:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tErrors Out:\t\t%" G_GUINT64_FORMAT "\n"
  //        "\tErrors Total:\t\t%" G_GUINT64_FORMAT "\n\n"
  //        "\tAddress6:\t\t%s\n"
  //        "\tPrefix6:\t\t%s\n"
  //        "\tScope6:\t\t\t%#03x\n\n"
  //        "\tHarware Address:\t%s\n\n",
  //        (unsigned long long)netload.flags,
  //        (unsigned long long)netload.if_flags, (guint32)netload.address,
  //        address_string, (guint32)netload.subnet, subnet_string, netload.mtu,
  //        netload.collisions, netload.packets_in, netload.packets_out,
  //        netload.packets_total, netload.bytes_in, netload.bytes_out,
  //        netload.bytes_total, netload.errors_in, netload.errors_out,
  //        netload.errors_total, address6_string, prefix6_string,
  //        (int)netload.scope6, hwaddress_string);

  // glibtop_close ();

  return G_SOURCE_CONTINUE;
}

void g_barbar_network_start(BarBarNetwork *net, gpointer data) {
  if (net->source_id > 0) {
    g_source_remove(net->source_id);
  }
  g_barbar_network_update(net);
  net->source_id =
      g_timeout_add_full(0, net->interval, g_barbar_network_update, net, NULL);
}
