import path from "node:path"
import fs from "node:fs"
import wav from "wav"

// TODO: Support audio pitch
// TODO: Support sound fade
// TODO: fix modifier keys triggers
// TODO: mute on fullscreen or secret mode
export default class AudioFont {
	constructor(id, device) {
		if (typeof id !== "string") {
			throw new Error("No id provided")
		}
		if (!device) {
			throw new Error("No device provided")
		}

		this.id = id
		this.device = device
		this.reader = new wav.Reader()

		this.reader.on("format", (format) => {
			this.format = format

			// (SampleRate * Channels * (Bits/8)) / 1000
			const bytesPerSample = format.bitDepth / 8
			const bytesPerSecond =
				format.sampleRate * format.channels * bytesPerSample

			this.bytesPerMillisecond = bytesPerSecond / 1000

			this.blockSize = format.channels * bytesPerSample
		})

		this.reader.on("data", (chunk) => {
			this.buffer = this.buffer
				? Buffer.concat([this.buffer, chunk])
				: chunk
		})
	}

	keys = {}
	module = null

	buffer = null
	format = null
	bytesPerMillisecond = 0
	blockSize = 0

	fileDataPath = null
	fileDataStream = null

	async load(fontPath) {
		if (!fs.existsSync(fontPath)) {
			throw new Error(`Font file ${fontPath} does not exist`)
		}

		let fontModule = await import(fontPath)

		fontModule = this.module = fontModule.default

		if (!fontModule.file) {
			throw new Error(`Font file ${fontPath} does not export a file`)
		}

		if (typeof fontModule.keys !== "object") {
			throw new Error(
				`Font file ${fontPath} does not export a keys object`,
			)
		}

		this.keys = fontModule.keys

		this.fileDataPath = path.resolve(
			path.dirname(fontPath),
			fontModule.file,
		)

		if (!fs.existsSync(this.fileDataPath)) {
			throw new Error(`Font file ${this.fileDataPath} does not exist`)
		}

		this.fileDataStream = fs.createReadStream(this.fileDataPath)
		this.fileDataStream.pipe(this.reader)

		await new Promise((resolve) => {
			this.reader.once("finish", (format) => {
				resolve()
			})
		})

		for (const key in this.keys) {
			this.keys[key].buff = this.getSpriteBuffer(key)
		}

		console.log(`[${this.id}] AudioFont loaded`, this.fileDataPath)
	}

	getSpriteBuffer(key) {
		const sprite = this.keys[key]

		if (!sprite) {
			return null
		}

		const startTimeMs = sprite[0]
		const durationMs = sprite[1]

		const startByte = Math.floor(startTimeMs * this.bytesPerMillisecond)
		const lengthByte = Math.floor(durationMs * this.bytesPerMillisecond)

		const alignedStart = startByte - (startByte % this.blockSize)

		if (alignedStart + lengthByte > this.buffer.length) {
			return null
		}

		return this.buffer.subarray(alignedStart, alignedStart + lengthByte)
	}

	play = (key) => {
		const buff = this.keys[key]?.buff ?? this.getSpriteBuffer(key)

		if (!buff) {
			console.error(`[${this.id}] AudioFont key ${key} not found`)
			return null
		}

		return this.device.player.play(buff)
	}

	handleTrigger = (event) => {
		if (!this.module.triggers || !Array.isArray(this.module.triggers)) {
			return null
		}

		const trigger = this.module.triggers.find((trigger) => {
			if (trigger.key_name !== event.key_name) {
				return false
			}

			if (trigger.shift && !event.shift) {
				return false
			}

			if (trigger.ctrl && !event.ctrl) {
				return false
			}

			if (trigger.alt && !event.alt) {
				return false
			}

			return true
		})

		if (trigger && trigger.value) {
			console.log(`[${this.id}] AudioFont trigger`, {
				key_name: event.key_name,
				value: trigger.value,
				shift: event.shift,
				ctrl: event.ctrl,
				alt: event.alt,
			})

			return this.play(trigger.value)
		}
	}
}
