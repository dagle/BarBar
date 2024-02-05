import BarBar from 'gi://BarBar';
import Gtk from 'gi://Gtk?version=4.0';
import GLib from 'gi://GLib';

let manager = [];

function activate(app) {
  let builder = new Gtk.Builder();

  builder.add_from_file("/home/dagle/.config/barbar/config.ui");

  let list = builder.get_objects();

  for (let obj of list) {
    if (obj instanceof BarBar.Bar) {
      app.add_window(obj);
      obj.present();
    } else if (obj instanceof BarBar.Sensor) {
      manager.push(obj);
      obj.start();
    }
  }
}

Gtk.init();
BarBar.init();

let app = new Gtk.Application({application_id: "com.github.barbar"});
app.connect("activate", activate);
app.run(ARGV);
