import ChildProcess from "child_process"

export default class Libinput {
	static listDevices() {
		let devices = ChildProcess.spawnSync("libinput", ["list-devices"])

		return Parser.devices(devices.stdout.toString())
	}

	static getKeyboards() {
		return getDevices().filter((device) =>
			device.capabilities.includes("keyboard"),
		)
	}

	static listenDeviceEvents(kernelPointer, onEvent) {
		const events = ChildProcess.exec(
			`libinput debug-events ${kernelPointer} --show-keycodes`,
		)

		events.stdout.on("data", (data) => {
			onEvent(Parser.event(data.toString()))
		})

		events.stderr.on("data", (data) => {
			throw new Error(data.toString())
		})
	}
}

export class Parser {
	static devices(input) {
		return input
			.trim()
			.split(/\n\s*\n/)
			.map((block) =>
				block.split("\n").reduce((acc, line) => {
					const splitIdx = line.indexOf(":")

					if (splitIdx === -1) {
						return acc
					}

					const key = line
						.slice(0, splitIdx)
						.trim()
						.toLowerCase()
						.replace(/[- ](\w)/g, (_, c) => c.toUpperCase())

					const val = line.slice(splitIdx + 1).trim()

					acc[key] = key === "capabilities" ? val.split(/\s+/) : val

					return acc
				}, {}),
			)
	}

	static event(event) {
		const regex =
			/(?<device>\S+)\s+(?<type>\S+)\s+\+(?<time>[0-9.]+)s\s+(?<key>\S+)\s+\((?<code>\d+)\)\s+(?<state>\w+)/

		const match = event.match(regex)

		if (match) {
			const data = {
				device: match.groups.device,
				event_type: match.groups.type,
				time: parseFloat(match.groups.time),
				key_name: match.groups.key,
				key_code: parseInt(match.groups.code),
				state: match.groups.state,
			}

			return data
		}

		return null
	}
}
