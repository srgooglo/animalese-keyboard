import gi from "node-gtk"

export default class GTKApp {
	constructor() {
		console.log(this.constructor)

		this.glib = gi.require("GLib", "2.0")
		this.gtk = gi.require("Gtk", "4.0")
		this.adw = gi.require("Adw", "1")
	}

	initialize() {
		this.mainLoop = this.glib.MainLoop.new(null, false)
		this.app = new this.adw.Application(
			this.constructor.appId,
			this.constructor.version,
		)

		this.app.on("activate", this._activate)
		this._run()
	}

	_activate = async () => {
		console.log("GTKApp::activate")

		if (typeof this.onActivate === "function") {
			await this.onActivate()
		}

		gi.startLoop()
		this.mainLoop.run()
	}

	_run = () => {
		console.log("GTKApp::run")
		this.app.run([])
		this.app.on("")
		console.log("GTKApp::run done")
	}
}
