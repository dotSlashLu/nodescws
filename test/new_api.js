var fs = require("fs");
var NodeScws = require("../build/Release/scws");
var dictDir = __dirname + "/../dicts",
    ruleDir = __dirname + "/../rules";

// throw error when called as a function
try {
  NodeScws();
}
catch (e) {
  console.log("should raise can't call as function error.");
  console.log(e);
  console.log();
}

try {
  new NodeScws();
}
catch (e) {
  console.log("should raise type error");
  console.log(e);
  console.log();
}

const conf = {
  charset: "utf8",
  debug: false,
  dicts: dictDir + "/dict.utf8.xdb:" + 
    dictDir + "/dict_cht.utf8.xdb:" + 
    dictDir + "/dict.test.txt",
  rule: ruleDir + "/rules.utf8.ini",
  ignorePunct: true,
  multi: "duality"
}
var content = fs.readFileSync(__dirname + "/test_doc.txt", {encoding: "utf8"});
var start = new Date().valueOf(), end;
var scws = new NodeScws(conf);
// console.log(scws.getConfig());
console.log(scws.segment(content));
console.log(scws.destroy());
end = new Date().valueOf();
console.log("time used: " + (end - start) + "ms");
