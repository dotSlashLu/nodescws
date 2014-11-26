var fs   = require("fs")
    Scws = require("../build/Release/scws");

fs.readFile(__dirname + "/test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  start = new Date().valueOf();
  // initialize scws with config entries
  var dictDir = __dirname + "/../dicts",
      ruleDir = __dirname + "/../rules";

  scws = new Scws.init({
    charset: "utf8",
    dicts: dictDir + "/dict.utf8.xdb:" + dictDir + "/dict_cht.utf8.xdb:" + dictDir + "/dict.test.txt",
    // dicts: "./dicts/dict.utf8.xdb",
    rule: ruleDir + "/rules.utf8.ini",
    // rule: "./rules/rules_cht.utf8.ini",
    ignorePunct: true,
    multi: "duality",
    debug: true,
    applyStopWord: true
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
})
