export default () => {
	const args = {}

	for (let i = 2; i < process.argv.length; i++) {
		const arg = process.argv[i]

		if (arg.startsWith("--")) {
			const eqPos = arg.indexOf("=")

			if (eqPos > -1) {
				const key = arg.substring(2, eqPos)
				const value = arg.substring(eqPos + 1)

				args[key] = value
			}
		}
	}

	return args
}
