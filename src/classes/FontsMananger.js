import fs from "node:fs"
import path from "node:path"
import AudioFont from "./AudioFont.js"

export default class FontManager {
	constructor({ fontsPath, audioDevice } = {}) {
		if (!fs.existsSync(fontsPath)) {
			throw new Error(`Fonts path [${fontsPath}] does not exist`)
		}

		if (!audioDevice) {
			throw new Error("No audio device provided")
		}

		this.audioDevice = audioDevice
		this.fontsPath = fontsPath
	}

	fonts = []

	async handleTriggerFromEvent(event) {
		for (const font of this.fonts) {
			font.handleTrigger(event)
		}
	}

	async initialize() {
		const fonts = await fs.promises.readdir(this.fontsPath)

		console.log("Loading fonts >", fonts)

		for (const fontId of fonts) {
			const fontPath = path.join(this.fontsPath, fontId)
			const fontManifestPath = path.join(fontPath, "font.mjs")

			if (!fs.existsSync(fontManifestPath)) {
				throw new Error(`Font ${fontId} does not have a font.mjs file`)
			}

			this.loadFontFromFilePath(fontId, fontManifestPath)
		}
	}

	async loadFontFromFilePath(fontId, fontFilePath) {
		if (typeof fontId !== "string") {
			throw new Error("No font id provided")
		}
		if (!fontFilePath) {
			throw new Error("No font file path provided")
		}

		if (!fs.existsSync(fontFilePath)) {
			throw new Error(`Font [${fontFilePath}] does not exist`)
		}

		let fontModule = require(fontFilePath)
		fontModule = this.module = fontModule.default

		if (!fontModule.file) {
			throw new Error(`Font file ${fontPath} does not export a file`)
		}

		const fontFileDataPath = path.resolve(
			path.dirname(fontFilePath),
			fontModule.file,
		)

		if (!fs.existsSync(fontFileDataPath)) {
			throw new Error(`Font file ${fontFileDataPath} does not exist`)
		}

		if (typeof fontModule.keys !== "object") {
			throw new Error(
				`Font file ${fontPath} does not export a keys object`,
			)
		}

		const font = new AudioFont({
			device: this.audioDevice,
			id: fontId,
			fileDataPath: fontFileDataPath,
			keys: fontModule.keys,
			triggers: fontModule.triggers,
			onTrigger: fontModule.onTrigger,
		})

		await font.load()

		this.fonts.push(font)
	}
}
