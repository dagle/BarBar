-- GTK4 only works with the git version of lgi. Sadly lgi hasn't made a release in years.
-- So you need the git version.

-- For GTK4 Layer Shell to get linked before libwayland-client we must explicitly load it before importing with lgi
-- If you use luajit you can do this.
--
-- local ffi = require("ffi")
-- ffi.cdef[[
--     void *dlopen(const char *filename, int flags);
-- ]]
-- ffi.C.dlopen("libgtk4-layer-shell.so", 0x101)

-- Now open the library normally with LGI
local lgi = require("lgi")
local Gtk = lgi.require("Gtk", "4.0")
local BarBar = lgi.require("BarBar", "1.0")
local GLib = lgi.require("GLib")

Gtk.init()
BarBar.init()

local manager = {}
local ui = GLib.get_user_config_dir() .. "/barbar/config.ui"

local app = Gtk.Application({
	application_id = "com.github.barbar",
})
app.on_activate = function()
	local builder = Gtk.Builder()

	builder:add_from_file(ui)

	local list = builder:get_objects()

	for _, obj in pairs(list) do
		if Gtk.Window:is_type_of(obj) then
			app:add_window(obj)
			obj:present()
		elseif BarBar.Sensor:is_type_of(obj) then
			table.insert(manager, obj)
			obj:start()
		end
	end
end
app:run()
