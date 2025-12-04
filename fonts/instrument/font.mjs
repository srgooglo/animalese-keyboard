export default {
	file: "data.wav",
	onTrigger: (event, trigger, font) => {
		const key = font.keys[trigger.value]

		if (!key) {
			return null
		}

		// copy the buffer
		let buff = key.buff

		// compute pitch multiplier from semitone offset
		// using the 2^(semitone/12) formula
		if (typeof trigger.semitone === "number") {
			buff = font.device.player.resample(
				key.buff,
				Math.pow(2, trigger.semitone / 12),
			)
		}

		// apply a fadeout
		buff = font.device.player.fadeout(buff, 100)

		// play the buffer
		font.device.player.play(buff)
	},
	triggers: [
		{ key_name: "KEY_1", value: "guitarDo", semitone: 0 },
		{ key_name: "KEY_2", value: "guitarDo", semitone: 2 },
		{ key_name: "KEY_3", value: "guitarDo", semitone: 4 },
		{ key_name: "KEY_4", value: "guitarDo", semitone: 5 },
		{ key_name: "KEY_5", value: "guitarDo", semitone: 7 },
		{ key_name: "KEY_6", value: "guitarDo", semitone: 9 },
		{ key_name: "KEY_7", value: "guitarDo", semitone: 11 },
		{ key_name: "KEY_8", value: "guitarDo", semitone: 12 },
		{ key_name: "KEY_9", value: "guitarDo", semitone: 14 },
		{ key_name: "KEY_0", value: "guitarDo", semitone: 16 },
	],
	keys: {
		guitarDo: [0, 300],
	},
}
