var fs   = require("fs")
    Scws = require("../build/Release/scws");

fs.readFile(__dirname + "/test_doc.txt", {
  encoding: "utf8"
}, function(err, data){
  if (err)
    return console.error(err);
  var start = new Date().valueOf();
  // scws config dir
  var dictDir = __dirname + "/../dicts",
      ruleDir = __dirname + "/../rules";

  fs.readFile(ruleDir + "/rules.utf8.json", {encoding: "utf8"}, function(err, setting){
    rule = JSON.stringify(JSON.parse(setting));
    var scws = new Scws.init({
      charset: "utf8",
      dicts: dictDir + "/dict.utf8.xdb:" + dictDir + "/dict_cht.utf8.xdb:" + dictDir + "/dict.test.txt",
      // dicts: "./dicts/dict.utf8.xdb",
      // rule: ruleDir + "/rules.utf8.json",
      rule: rule,
      // rule: "./rules/rules_cht.utf8.ini",
      ignorePunct: true,
      multi: "duality",
      debug: false,
      applyStopWord: true
    });
    // segment text
    var res = scws.segment(data),
        end = new Date().valueOf();
    console.log("time used: " + (end - start) + "ms");
    console.log("top results from text: ", res.splice(0, 10));
    // destroy scws, recollect memory
    scws.destroy();
  })
})
