{
  "targets": [
    {
      "target_name": "scws",
        "sources": [
          "./src/nodescws.cc",
          "./src/scws.cc",
          "<!@(ls -1 ./src/libscws/*.c)",
          "./src/cjson/cJSON.c"
        ]
    }
  ],
}
