import BingusMixer from "../../lib/mixer/mixer.node"

export default class AudioDevice {
	player = null

	initialize() {
		this.player = new BingusMixer.Mixer("/run/user/1000/pulse/native")
		console.log("AudioDevice initialized")
	}
}
