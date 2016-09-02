var NodeScws = require("../build/Release/scws");

// throw error when called as a function
try {
        NodeScws();
}
catch (e) {
        console.log(e);
}

var scws = new NodeScws({a: 1});
console.log(scws.getConfig());
// console.log(scws.config());
