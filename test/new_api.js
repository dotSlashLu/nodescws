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

try {
  var scws = new NodeScws({});
  scws.destroy();
  scws.segment("abc");
}
catch (e) {
  console.log("should raise error: already destroy()ed");
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
var content = fs.readFileSync(__dirname + "/test_doc.txt", {encoding: "utf8"}),
    content_en = "引言 VMwareTools 是 VMware 虚拟机中很重要的一个工具包，有些时候在虚拟机中安装完操作系统会缺少网卡驱动，不能上网，这时只要安装 VMwareTools 就可以解决问题，下面以 CentOS 为例，来说明 VMwareTools 的安装方法";
var start = new Date().valueOf(), end;
var scws = new NodeScws(conf);
// console.log(scws.getConfig());
console.log(scws.segment(content));
console.log(scws.segment(content_en));
console.log(scws.destroy());
end = new Date().valueOf();
console.log("time used: " + (end - start) + "ms");
