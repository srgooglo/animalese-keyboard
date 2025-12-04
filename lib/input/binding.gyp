{
  "targets": [
    {
      "target_name": "input",
      "sources": [ "src/addon.c", "src/main.c" ],
      "include_dirs": [
        "src"
      ],
      "libraries": [
        "-levdev",
        "-linput",
        "-ludev",
        "-lpthread"
      ]
    }
  ]
}
