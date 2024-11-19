import BarBar from 'gi://BarBar';
import Gtk from 'gi://Gtk?version=4.0';
import Systray from 'gi://Statusnotifier';
import GObject from 'gi://GObject';
import GLib from 'gi://GLib';

let manager = [];

function newscreen(head, app) {

  // head.connect('notify::resolution-height', (source, paramSpec) => {
  //   print(`The property ${paramSpec.name} has changed to: ${source.resolutionHeight}`);
  // });
  print(head.get_resolution_width());
  if (head.get_name() == "eDP-1") {
    let builder = BarBar.default_builder("barbar/config.ui");

    let list = builder.get_objects();

    for (let obj of list) {
      if (obj instanceof Gtk.Window) {
        app.add_window(obj);
        obj.present();
      } else if (obj instanceof BarBar.Sensor) {
        manager.push(obj);
        obj.start();
      } else if (obj instanceof Gtk.ScaleButton) {
        let minus = obj.get_minus_button();
        let plus = obj.get_plus_button();
        minus.set_visible(false);
        plus.set_visible(false);
      }
    }
  }
}

Gtk.init();
BarBar.init();
// Systray doesn't have a init function
GObject.type_ensure(Systray.Systray);

function activate(app) {
  BarBar.default_style_provider("barbar/style.css");

  let context = GLib.MainContext.default();
  let outputs = new BarBar.OutputManager();

  outputs.connect("new-head", (_, head) => {
    newscreen(head, app);
  });
  outputs.run();

}

let app = new Gtk.Application({ application_id: "com.github.barbar" });
app.connect("activate", activate);
app.run(ARGV);
