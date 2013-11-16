var fs   = require("fs");

fs.readFile("./test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  start = new Date().valueOf();
  var scws = require("./build/Release/scws");
  var res = scws.segment(data, {
    charset: "utf8",
    //dicts: "./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb:./dicts/dict.test.txt",
    dicts: "./dicts/dict.utf8.xdb",
    rule: "./rules/rules.utf8.ini",
    ignorePunct: true,
    // multi: "duality"
  });
  end = new Date().valueOf();
  console.log("time used: " + (end - start));
  console.log("top results: ", res.splice(0, 60));
})
