import BarBar from 'gi://BarBar';
import Gtk from 'gi://Gtk?version=4.0';
import Vte from 'gi://Vte?version=3.91';
import GLib from 'gi://GLib?version=2.0';

function activate(app) {

  let background = new BarBar.Background({
    top_margin: 45,
    bottom_margin: 30,
    right_margin: 200,
    left_margin: 200
  });
  let term = new Vte.Terminal();
  term.spawn_async(Vte.PtyFlags.DEFAULT, null, ['/bin/nvim'],
    null, GLib.SpawnFlags.DEFAULT, null, -1, null, null);
  background.set_child(term);
  app.add_window(background);
  background.present();
}

Gtk.init();
BarBar.init();

let app = new Gtk.Application({ application_id: "com.github.barbar" });
app.connect("activate", activate);
app.run(ARGV);
