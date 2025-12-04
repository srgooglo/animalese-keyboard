import Libinput from "./classes/Libinput.js"
import AudioDevice from "./classes/AudioDevice.js"
import AudioFont from "./classes/AudioFont.js"

import path from "node:path"
const fonts_path = path.join(import.meta.dirname, "fonts")

let withShift = false
let withAlt = false
let withCtrl = false

// TODO: fix modifier keys triggers
async function main() {
	if (!process.argv[2]) {
		console.error("Usage: node main.js <device>")
		return false
	}

	// first create the device
	const audioDevice = new AudioDevice()
	audioDevice.initialize()

	// create the default voice font
	const voiceFont = new AudioFont("voice", audioDevice)
	await voiceFont.load(path.join(fonts_path, "voices", "font.js"))

	// create the sfx font
	const sfxFont = new AudioFont("sfx", audioDevice)
	await sfxFont.load(path.join(fonts_path, "sfx", "font.js"))

	// create the event listener
	Libinput.listenDeviceEvents(process.argv[2], (event) => {
		if (!event) {
			return null
		}

		if (
			event.key_name === "KEY_LEFTSHIFT" ||
			event.key_name === "KEY_RIGHTSHIFT"
		) {
			withShift = event.state === "pressed"
		}
		if (
			event.key_name === "KEY_LEFTCTRL" ||
			event.key_name === "KEY_RIGHTCTRL"
		) {
			withCtrl = event.state === "pressed"
		}
		if (
			event.key_name === "KEY_LEFTALT" ||
			event.key_name === "KEY_RIGHTALT"
		) {
			withAlt = event.state === "pressed"
		}

		console.log(event)

		if (event.state === "pressed") {
			voiceFont.handleTrigger({
				...event,
				shift: withShift,
				ctrl: withCtrl,
				alt: withAlt,
			})

			sfxFont.handleTrigger({
				...event,
				shift: withShift,
				ctrl: withCtrl,
				alt: withAlt,
			})
		}
	})
}

main()
