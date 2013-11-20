var fs   = require("fs")
    Scws = require("./build/Release/scws");

fs.readFile("./test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  start = new Date().valueOf();
  // initialize scws with config entries
  scws = new Scws.init({
    charset: "utf8",
    //dicts: "./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb:./dicts/dict.test.txt",
    dicts: "./dicts/dict.utf8.xdb",
    rule: "./rules/rules.utf8.ini",
    ignorePunct: true,
    multi: "duality",
    debug: true
  });
  // segment text
  res = scws.segment(data);
  res1 = scws.segment("大家好我来自日本，我是日本人");
  res2 = scws.segment("大家好我来自德国，我是德国人");
  end = new Date().valueOf();
  console.log("time used: " + (end - start) + "ms");
  console.log("top results from text: ", res.splice(0, 60));
  console.log("test reuse of scws: ", res1);
  console.log("test reuse of scws2: ", res2);
  // destroy scws, recollect memory
  scws.destroy();
>>>>>>> develop
})
