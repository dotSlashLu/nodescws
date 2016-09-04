var NodeScws = require("../build/Release/scws");

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

var scws = new NodeScws({a: 1});
var conf = scws.getConfig()
console.log(typeof(conf), conf);
// console.log(scws.config());
