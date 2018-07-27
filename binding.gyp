{
  "targets": [
    {
      "target_name": "scws",
      "sources": [
        "./src/nodescws.cc",
        "./src/scws.cc",
        "./src/libscws/charset.c",
        "./src/libscws/crc32.c",
        "./src/libscws/darray.c",
        "./src/libscws/lock.c",
        "./src/libscws/pool.c",
        "./src/libscws/rule.c",
        "./src/libscws/scws.c",
        "./src/libscws/xdb.c",
        "./src/libscws/xdict.c",
        "./src/libscws/xtree.c",
        "./src/cjson/cJSON.c"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
