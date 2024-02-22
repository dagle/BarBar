import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import Gdk from 'gi://Gdk?version=3.0';
import Gtk from 'gi://Gtk?version=3.0';
import GdkPixbuf from 'gi://GdkPixbuf';
// import DbusmenuGtk3 from 'gi://DbusmenuGtk3';
// import Service from '../service.js';

function loadInterfaceXML(iface) {
  // const uri = `resource:///com/github/Aylur/ags/dbus/${iface}.xml`;
  // const f = Gio.File.new_for_uri(uri);
  const f = Gio.File.new_for_path(`protocol/${iface}.xml`);

  try {
    const [, bytes] = f.load_contents(null);
    return new TextDecoder().decode(bytes);
  } catch (e) {
    logError(e);
    return null;
  }
}

const StatusNotifierWatcherIFace = loadInterfaceXML('dbus-status-notifier-watcher');
const StatusNotifierItemIFace = loadInterfaceXML('dbus-status-notifier-item');
const DbusMenu = loadInterfaceXML('dbus-menu');
const StatusNotifierItemProxy = Gio.DBusProxy.makeProxyWrapper(StatusNotifierItemIFace);
const DbusMenuProxy = Gio.DBusProxy.makeProxyWrapper(DbusMenu);

// export class TrayItem extends Service {
//   static {
//     Service.register(this, {
//       'removed': ['string'],
//       'ready': [],
//     }, {
//       'menu': ['jsobject'],
//       'category': ['string'],
//       'id': ['string'],
//       'title': ['string'],
//       'status': ['string'],
//       'window-id': ['int'],
//       'is-menu': ['boolean'],
//       'tooltip-markup': ['string'],
//       'icon': ['jsobject'],
//     });
//   }
//
//   private _proxy: StatusNotifierItemProxy;
//   private _busName: string;
//
//   private _iconTheme?: Gtk.IconTheme;
//   menu?: DbusmenuGtk3.Menu;
//
//   constructor(busName: string, objectPath: string) {
//     super();
//
//     this._busName = busName;
//
//     this._proxy = new StatusNotifierItemProxy(
//       Gio.DBus.session,
//       busName,
//       objectPath,
//       this._itemProxyAcquired.bind(this),
//       null,
//       Gio.DBusProxyFlags.NONE);
//   }
//
//   readonly activate = (event: Gdk.Event) => {
//     this._proxy.ActivateAsync(event.get_root_coords()[1], event.get_root_coords()[2]);
//   };
//
//   readonly secondaryActivate = (event: Gdk.Event) => {
//     this._proxy.SecondaryActivateAsync(event.get_root_coords()[1], event.get_root_coords()[2]);
//   };
//
//   readonly scroll = (event: Gdk.EventScroll) => {
//     const direction = (event.direction == 0 || event.direction == 1)
//       ? 'vertical' : 'horizontal';
//
//     const delta = (event.direction == 0 || event.direction == 1)
//       ? event.delta_y : event.delta_x;
//
//     this._proxy.ScrollAsync(delta, direction);
//   };
//
//   readonly openMenu = (event: Gdk.Event) => {
//     this.menu
//       ? this.menu.popup_at_pointer(event)
//       : this._proxy.ContextMenuAsync(event.get_root_coords()[1], event.get_root_coords()[2]);
//   };
//
//   get category() { return this._proxy.Category; }
//   get id() { return this._proxy.Id; }
//   get title() { return this._proxy.Title; }
//   get status() { return this._proxy.Status; }
//   get window_id() { return this._proxy.WindowId; }
//   get is_menu() { return this._proxy.ItemIsMenu; }
//
//   get tooltip_markup() {
//     if (!this._proxy.ToolTip)
//       return '';
//
//     let tooltipMarkup = this._proxy.ToolTip[2];
//     if (this._proxy.ToolTip[3] !== '')
//       tooltipMarkup += '\n' + this._proxy.ToolTip[3];
//
//     return tooltipMarkup;
//   }
//
//   get icon() {
//     const iconName = this.status === 'NeedsAttention'
//       ? this._proxy.AttentionIconName
//       : this._proxy.IconName;
//
//     if (this._iconTheme && iconName) {
//       const size = Math.max(...this._iconTheme.get_icon_sizes(iconName));
//       const iconInfo = this._iconTheme.lookup_icon(
//         iconName, size, Gtk.IconLookupFlags.FORCE_SIZE);
//
//       if (iconInfo)
//         return iconInfo.load_icon();
//     }
//     const iconPixmap = this.status === 'NeedsAttention'
//       ? this._proxy.AttentionIconPixmap
//       : this._proxy.IconPixmap;
//
//     return iconName || this._getPixbuf(iconPixmap) || 'image-missing';
//   }
//
//   private _itemProxyAcquired(proxy: StatusNotifierItemProxy) {
//     if (proxy.Menu) {
//       const menu = DbusmenuGtk3Menu({
//         dbus_name: proxy.g_name_owner!,
//         dbus_object: proxy.Menu,
//       });
//       this.menu = (menu as unknown) as DbusmenuGtk3.Menu;
//     }
//
//     if (this._proxy.IconThemePath) {
//       this._iconTheme = Gtk.IconTheme.new();
//       this._iconTheme?.set_search_path([this._proxy.IconThemePath]);
//     }
//
//     bulkConnect(proxy, [
//       ['notify::g-name-owner', () => {
//         if (!proxy.g_name_owner)
//           this.emit('removed', this._busName);
//       }],
//       ['g-signal', this._refreshAllProperties.bind(this)],
//       ['g-properties-changed', () => this.emit('changed')],
//     ]);
//
//     ['Title', 'Icon', 'AttentionIcon', 'OverlayIcon', 'ToolTip', 'Status']
//       .forEach(prop => proxy.connectSignal(`New${prop}`, () => {
//         this._notify();
//       }));
//
//     this.emit('ready');
//   }
//
//   private _notify() {
//     [
//       'menu', 'category', 'id', 'title', 'status',
//       'window-id', 'is-menu', 'tooltip-markup', 'icon',
//     ].forEach(prop => this.notify(prop));
//     this.emit('changed');
//   }
//
//   private _refreshAllProperties() {
//     this._proxy.g_connection.call(
//       this._proxy.g_name,
//       this._proxy.g_object_path!,
//       'org.freedesktop.DBus.Properties',
//       'GetAll',
//       new GLib.Variant('(s)', [this._proxy.g_interface_name]),
//       new GLib.VariantType('(a{sv})'),
//       Gio.DBusCallFlags.NONE, -1,
//       null,
//       (proxy, result) => {
//         const variant = proxy?.call_finish(result) as GLib.Variant;
//         if (!variant)
//           return;
//         const [properties] = variant.deepUnpack < Record < string, GLib.Variant> [] > ();
//         Object.entries(properties).map(([propertyName, value]) => {
//           this._proxy.set_cached_property(propertyName, value);
//         });
//
//         if (this._proxy.IconThemePath) {
//           if (!this._iconTheme)
//             this._iconTheme = Gtk.IconTheme.new();
//
//           this._iconTheme.set_search_path([this._proxy.IconThemePath]);
//         }
//
//         this._notify();
//       },
//     );
//   }
//
//   private _getPixbuf(pixMapArray: [number, number, Uint8Array][]) {
//     if (!pixMapArray)
//       return;
//
//     const pixMap = pixMapArray.sort((a, b) => a[0] - b[0]).pop();
//     if (!pixMap)
//       return;
//
//     const array = Uint8Array.from(pixMap[2]);
//     for (let i = 0; i < 4 * pixMap[0] * pixMap[1]; i += 4) {
//       const alpha = array[i];
//       array[i] = array[i + 1];
//       array[i + 1] = array[i + 2];
//       array[i + 2] = array[i + 3];
//       array[i + 3] = alpha;
//     }
//     return GdkPixbuf.Pixbuf.new_from_bytes(
//       new GLib.Bytes(array),
//       GdkPixbuf.Colorspace.RGB,
//       true,
//       8,
//       pixMap[0],
//       pixMap[1],
//       pixMap[0] * 4,
//     );
//   }
// }

// export class SystemTray extends Service {
//   static {
//     Service.register(this, {
//       'added': ['string'],
//       'removed': ['string'],
//     }, {
//       'items': ['jsobject'],
//     });
//   }
//
//   private _dbus!: Gio.DBusExportedObject;
//   private _items: Map<string, TrayItem>;
//
// get IsStatusNotifierHostRegistered() { return true; }
// get ProtocolVersion() { return 0; }
// get RegisteredStatusNotifierItems() { return Array.from(this._items.keys()); }
//
// get items() { return Array.from(this._items.values()); }
// readonly getItem = (name: string) => this._items.get(name);
//
// constructor() {
//   super();
//   this._items = new Map();
//   this._register();
// }

// private _register() {

class TrayItem {
  constructor(busName, objectPath) {

    this._busName = busName;
    this._objectPath = objectPath;

    this._proxy = new StatusNotifierItemProxy(
      Gio.DBus.session,
      busName,
      objectPath,
      this._itemProxyAcquired.bind(this),
      null,
      Gio.DBusProxyFlags.NONE);
  }

  _dbus_menu(menu) {
    // console.log("menu: ", menu);
    try {
      let [revison, layout] = menu.GetLayoutSync(0, -1, ["type", "label", "visible", "enabled", "children-display", "accessible-desc"]);
      // let obj = layout[1][1];
      // console.log("layout:", layout[2][2].print(true));
    } catch (e) {
      logError(e);
    }
  }

  _itemProxyAcquired(proxy) {
    if (proxy.Menu) {
      try {
        log(proxy.IconName);
        let theme = Gtk.IconTheme.get_default();
        let icon = theme.lookup_icon(proxy.IconName, 48, 1);
        log(icon)
        // let icon = GdkPixbuf.Pixbuf.new_from_file_at_size(proxy.IconName,
        //   16,
        //   16
        // );
        this._menu = new DbusMenuProxy(
          Gio.DBus.session,
          proxy.g_name_owner,
          proxy.Menu,
          this._dbus_menu.bind(this),
          null,
          Gio.DBusProxyFlags.NONE);
        //     const menu = DbusmenuGtk3Menu({
        //       dbus_name: proxy.g_name_owner,
        //       dbus_object: proxy.Menu,
        // });
        //     // this.menu = (menu as unknown) as DbusmenuGtk3.Menu;
      } catch (e) {
        logError(e);
      }
    }
    //
    //   if (this._proxy.IconThemePath) {
    //     this._iconTheme = Gtk.IconTheme.new();
    //     this._iconTheme?.set_search_path([this._proxy.IconThemePath]);
    //   }
    //
    //   bulkConnect(proxy, [
    //     ['notify::g-name-owner', () => {
    //       if (!proxy.g_name_owner)
    //         this.emit('removed', this._busName);
    //     }],
    //     ['g-signal', this._refreshAllProperties.bind(this)],
    //     ['g-properties-changed', () => this.emit('changed')],
    //   ]);
    //
    //   ['Title', 'Icon', 'AttentionIcon', 'OverlayIcon', 'ToolTip', 'Status']
    //     .forEach(prop => proxy.connectSignal(`New${prop}`, () => {
    //       this._notify();
    //     }));
    //
    //   this.emit('ready');
  }
}
class Service {
  RegisterStatusNotifierItemAsync(serviceName, invocation) {
    let busName = null;
    let objectPath = null;
    const [service] = serviceName;
    if (service.startsWith('/')) {
      objectPath = service;
      busName = invocation.get_sender();
    } else {
      busName = service;
      objectPath = '/StatusNotifierItem';
    }

    invocation.return_value(null);

    const item = new TrayItem(busName, objectPath);
    // item.connect('ready', () => {
    //   this._items.set(busName, item);
    //   this.emit('added', busName);
    //   this.notify('items');
    //   this.emit('changed');
    //   this._dbus.emit_signal(
    //     'StatusNotifierItemRegistered',
    //     new GLib.Variant('(s)', [busName + objectPath]),
    //   );
    // });
    // item.connect('removed', () => {
    //   this._items.delete(busName);
    //   this.emit('removed', busName);
    //   this.notify('items');
    //   this.emit('changed');
    //   this._dbus.emit_signal(
    //     'StatusNotifierItemUnregistered',
    //     new GLib.Variant('(s)', [busName]),
    //   );
    // });
  }
  RegisterStatusNotifierHostAsync(serviceName, invocation) {
    console.log("blerp");
  }
}

function activate(app) {
  let serviceInstance = new Service();

  try {
    StatusNotifierWatcherIFace.bus_own_name();
    Gio.bus_own_name(
      Gio.BusType.SESSION,
      'org.kde.StatusNotifierWatcher',
      Gio.BusNameOwnerFlags.NONE,
      (connection) => {
        let dbus = Gio.DBusExportedObject
          .wrapJSObject(StatusNotifierWatcherIFace, serviceInstance);

        dbus.export(connection, '/StatusNotifierWatcher');
      },
      null,
      () => {
        print('Another system tray is already running');
      },
    );
  } catch (e) {
    log(e);
  }

  let win = new Gtk.Window();
  app.add_window(win);
  // win.present();

}

let app = new Gtk.Application({ application_id: "com.github.barbar" });
app.connect("activate", activate);
app.run(ARGV);
// }

//
// export const systemTray = new SystemTray;
// export default systemTray;
