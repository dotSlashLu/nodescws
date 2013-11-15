    fs   = require("fs");
/*
 * scws(text, charset, dict, ignore_punct, multi);
 * */
fs.readFile("./test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  start = new Date().valueOf();
  var scws = require("./build/Release/scws");
  // console.log("going to segment:" + data);
  var res = scws.segment(data, "utf8", "./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb");
  end = new Date().valueOf();
  console.log("time used: " + (end - start));
  console.log("10 result: ", res.splice(0, 10));
})
