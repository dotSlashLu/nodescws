var scws = require('./build/Release/scws');
/*
 * scws(text, charset, dict, ignore_punct, multi);
 * */
console.log(scws.segment("大家好，我是一个程序员，这个项目是由scws port的node addon。"));
