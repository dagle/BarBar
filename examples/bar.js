import BarBar from 'gi://BarBar';
import Gtk from 'gi://Gtk?version=4.0';
import Vte from 'gi://Vte?version=3.91';
import GLib from 'gi://GLib?version=2.0';

let manager = [];

function activate(app) {

  BarBar.default_style_provider("barbar/style.css");
  let builder = BarBar.default_builder("barbar/config.ui");

  let term = new Vte.Terminal();
  term.spawn_async(Vte.PtyFlags.DEFAULT, null, ['/bin/nvim'],
    null, GLib.SpawnFlags.DEFAULT, null, -1, null, null);
  let background = builder.get_object("background");
  background.set_child(term);

  let list = builder.get_objects();

  for (let obj of list) {
    if (obj instanceof Gtk.Window) {
      app.add_window(obj);
      obj.present();
    } else if (obj instanceof BarBar.Sensor) {
      manager.push(obj);
      obj.start();
    } else if (obj instanceof Gtk.ScaleButton){
      let minus = obj.get_minus_button();
      let plus = obj.get_plus_button();
      minus.set_visible(false);
      plus.set_visible(false);
    }
  }
}

Gtk.init();
BarBar.init();

let app = new Gtk.Application({ application_id: "com.github.barbar" });
app.connect("activate", activate);
app.run(ARGV);
