import GTKApp from "./classes/GTKApp.js"

export default class UI extends GTKApp {
	static appId = "net.ragestudio.gtktest"
	static defaultSize = [200, 200]
	static defaultTitle = "GTK4 Test"
	static version = "1.0.0"

	handleTestClick = () => {
		console.log("Hello World")
	}

	onActivate = () => {
		this.window = new this.gtk.ApplicationWindow(this.app)
		this.window.setTitle(UI.defaultTitle)
		this.window.setDefaultSize(UI.defaultSize[0], UI.defaultSize[1])

		this.window.on("close-request", this.onQuit)

		const ui = readView("main")

		const builder = this.gtk.Builder.newFromString(ui, ui.length)
		const root = builder.getObject("root")

		const actionButton = builder.getObject("actionButton")
		actionButton.on("clicked", this.handleTestClick)

		const closeButton = builder.getObject("closeButton")
		closeButton.on("clicked", () => this.window.close())

		this.window.setChild(root)
		this.window.show()
		this.window.present()
	}

	onQuit = () => {
		console.log("Quitting")
		return false
	}
}
