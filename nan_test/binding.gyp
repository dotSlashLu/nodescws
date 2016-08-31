{
  "targets": [
    {
      "target_name": "addon-obj",
      "sources": [ "addon-obj.cc", "obj.cc" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
