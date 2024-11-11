import BarBar from 'gi://BarBar';
import Gtk from 'gi://Gtk?version=4.0';
import Systray from 'gi://Statusnotifier';
import GObject from 'gi://GObject';

let manager = [];

function activate(app) {
  let manager = new BarBar.OutputManger();

  // BarBar.default_style_provider("barbar/style.css");
  // let builder = BarBar.default_builder("barbar/config.ui");
  //
  // let list = builder.get_objects();
  //
  // for (let obj of list) {
  //   if (obj instanceof Gtk.Window) {
  //     app.add_window(obj);
  //     obj.present();
  //   } else if (obj instanceof BarBar.Sensor) {
  //     manager.push(obj);
  //     obj.start();
  //   } else if (obj instanceof Gtk.ScaleButton) {
  //     let minus = obj.get_minus_button();
  //     let plus = obj.get_plus_button();
  //     minus.set_visible(false);
  //     plus.set_visible(false);
  //   }
  // }
}

Gtk.init();
BarBar.init();
// Systray doesn't have a init function
GObject.type_ensure(Systray.Systray);
for (let key in BarBar) {
    // Check if the property is a class (GObject type)
    if (BarBar[key] && BarBar[key].$gtype && GObject.type_is_a(BarBar[key].$gtype, GObject.Object.$gtype)) {
        print(`${key} is a class`);
    }
}

let app = new Gtk.Application({ application_id: "com.github.barbar" });
app.connect("activate", activate);
app.run(ARGV);
