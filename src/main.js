import Libinput from "./classes/Libinput.js"
import AudioDevice from "./classes/AudioDevice.js"
import FontsManager from "./classes/FontsMananger.js"

import getArgs from "./utils/getArgs.js"

let withShift = false
let withAlt = false
let withCtrl = false

// TODO: mute on fullscreen or secret mode
// TODO: support contiuous trigger
// TODO: add system tray
async function main() {
	const args = getArgs()

	const { fonts: fontsPath } = args

	if (!fontsPath) {
		console.error("No fonts path provided, use --fonts=/path/to/fonts")
		return false
	}

	// first create the device
	const audioDevice = new AudioDevice()
	audioDevice.initialize()

	const fontsManager = new FontsManager({
		fontsPath: fontsPath,
		audioDevice: audioDevice,
	})
	await fontsManager.initialize()

	// create the event listener
	Libinput.listen((event) => {
		if (!event) {
			return null
		}

		console.log(event)

		if (
			event.keyname === "KEY_LEFTSHIFT" ||
			event.keyname === "KEY_RIGHTSHIFT"
		) {
			withShift = event.pressed
		}
		if (
			event.keyname === "KEY_LEFTCTRL" ||
			event.keyname === "KEY_RIGHTCTRL"
		) {
			withCtrl = event.pressed
		}
		if (
			event.keyname === "KEY_LEFTALT" ||
			event.keyname === "KEY_RIGHTALT"
		) {
			withAlt = event.pressed
		}

		event.shift = withShift
		event.ctrl = withCtrl
		event.alt = withAlt

		if (event.pressed) {
			fontsManager.handleTriggerFromEvent(event)
		}
	})
}

main()
