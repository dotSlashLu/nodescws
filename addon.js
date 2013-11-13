var scws = require("./build/Release/scws"),
    fs   = require("fs");
/*
 * scws(text, charset, dict, ignore_punct, multi);
 * */
fs.readFile("./test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  // console.log("going to segment:" + data);
  var res = scws.segment(data);
  console.log(res);
})
