const letterDuration = 200

export default {
	file: "data.wav",
	onTrigger: (event, trigger, font) => {
		const key = font.keys[trigger.value]

		if (!key || !key.buff) {
			return null
		}

		// copy the buffer
		let buff = key.buff

		// const randomShift = parseFloat((Math.random() * 0.1 + 0.95).toFixed(2))

		// // apply a random small pitch shift
		// buff = font.device.player.resample(key.buff, randomShift)

		// apply a fadeout
		buff = font.device.player.fadeout(buff, 100)

		// play the buffer
		font.device.player.play(buff)
	},
	triggers: [
		{ key_name: "KEY_1", value: "gwah", shift: true },
		{ key_name: "KEY_A", value: "a" },
		{ key_name: "KEY_B", value: "b" },
		{ key_name: "KEY_C", value: "c" },
		{ key_name: "KEY_D", value: "d" },
		{ key_name: "KEY_E", value: "e" },
		{ key_name: "KEY_F", value: "f" },
		{ key_name: "KEY_G", value: "g" },
		{ key_name: "KEY_H", value: "h" },
		{ key_name: "KEY_I", value: "i" },
		{ key_name: "KEY_J", value: "j" },
		{ key_name: "KEY_K", value: "k" },
		{ key_name: "KEY_L", value: "l" },
		{ key_name: "KEY_M", value: "m" },
		{ key_name: "KEY_N", value: "n" },
		{ key_name: "KEY_O", value: "o" },
		{ key_name: "KEY_P", value: "p" },
		{ key_name: "KEY_Q", value: "q" },
		{ key_name: "KEY_R", value: "r" },
		{ key_name: "KEY_S", value: "s" },
		{ key_name: "KEY_T", value: "t" },
		{ key_name: "KEY_U", value: "u" },
		{ key_name: "KEY_V", value: "v" },
		{ key_name: "KEY_W", value: "w" },
		{ key_name: "KEY_X", value: "x" },
		{ key_name: "KEY_Y", value: "y" },
		{ key_name: "KEY_Z", value: "z" },
	],
	keys: {
		a: [0, letterDuration],
		b: [letterDuration * 1, letterDuration],
		c: [letterDuration * 2, letterDuration],
		d: [letterDuration * 3, letterDuration],
		e: [letterDuration * 4, letterDuration],
		f: [letterDuration * 5, letterDuration],
		g: [letterDuration * 6, letterDuration],
		h: [letterDuration * 7, letterDuration],
		i: [letterDuration * 8, letterDuration],
		j: [letterDuration * 9, letterDuration],
		k: [letterDuration * 10, letterDuration],
		l: [letterDuration * 11, letterDuration],
		m: [letterDuration * 12, letterDuration],
		n: [letterDuration * 13, letterDuration],
		o: [letterDuration * 14, letterDuration],
		p: [letterDuration * 15, letterDuration],
		q: [letterDuration * 16, letterDuration],
		r: [letterDuration * 17, letterDuration],
		s: [letterDuration * 18, letterDuration],
		t: [letterDuration * 19, letterDuration],
		u: [letterDuration * 20, letterDuration],
		v: [letterDuration * 21, letterDuration],
		w: [letterDuration * 22, letterDuration],
		x: [letterDuration * 23, letterDuration],
		y: [letterDuration * 24, letterDuration],
		z: [letterDuration * 25, letterDuration],
		ok: [600 * 0 + 200 * 26, 600],
		gwah: [600 * 1 + 200 * 26, 600],
		deska: [600 * 2 + 200 * 26, 600],
	},
}
