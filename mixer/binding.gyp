{
  "targets": [
    {
      "target_name": "mixer",
      "sources": [
        "src/addon.cc",
        "src/mixer.c"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "src"
      ],
      "libraries": [
        "-lpulse",
        "-lpulse-simple"
      ],
      "cflags": [
        "<!@(pkg-config --cflags libpulse)"
      ],
      "conditions": [
        ['OS=="linux"', {
           "link_settings": {
             "libraries": ["-lpulse"]
           }
        }]
      ]
    }
  ]
}
