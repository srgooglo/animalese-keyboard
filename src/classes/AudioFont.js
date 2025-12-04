import fs from "node:fs"
import wav from "wav"

export default class AudioFont {
	constructor({ id, device, fileDataPath, keys, triggers, onTrigger } = {}) {
		if (typeof id !== "string") {
			throw new Error("No id provided")
		}
		if (!device) {
			throw new Error("No device provided")
		}
		if (!fileDataPath) {
			throw new Error("No file data path provided")
		}

		this.id = id
		this.device = device
		this.keys = keys ?? {}
		this.triggers = triggers ?? []
		this.onTrigger = onTrigger
		this.fileDataPath = fileDataPath

		// create the wav reader
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
	triggers = []
	onTrigger = null

	fileDataPath = null
	fileDataStream = null

	buffer = null
	format = null
	bytesPerMillisecond = 0
	blockSize = 0

	async load() {
		console.log(`[${this.id}] Loading AudioFont`)

		if (!this.fileDataPath) {
			throw new Error(`[${this.id}] No file data path provided`)
		}

		this.fileDataStream = fs.createReadStream(this.fileDataPath)
		this.fileDataStream.pipe(this.reader)

		await new Promise((resolve) => {
			this.reader.once("finish", (format) => {
				resolve()
			})
		})

		// precompute the sprite buffers
		for (const key in this.keys) {
			this.keys[key].buff = this.getSpriteBuffer(key)
		}

		console.log(`[${this.id}] AudioFont loaded`, this.fileDataPath)
	}

	getSpriteBuffer(key) {
		if (!this.buffer) {
			return null
		}

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
		if (!this.triggers || !Array.isArray(this.triggers)) {
			return null
		}

		const trigger = this.triggers.find((trigger) => {
			if (trigger.key_name !== event.keyname) {
				return false
			}

			if (!!trigger.shift !== !!event.shift) {
				return false
			}

			if (!!trigger.ctrl !== !!event.ctrl) {
				return false
			}

			if (!!trigger.alt !== !!event.alt) {
				return false
			}

			return true
		})

		if (trigger && trigger.value) {
			console.log(`[${this.id}] AudioFont trigger`, {
				key_name: event.keyname,
				value: trigger.value,
				shift: event.shift,
				ctrl: event.ctrl,
				alt: event.alt,
			})

			if (typeof this.onTrigger === "function") {
				return this.onTrigger(event, trigger, this)
			}

			return this.play(trigger.value)
		}
	}
}
