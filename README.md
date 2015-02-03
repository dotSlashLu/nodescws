# nodescws

### scws

##### About
scws即Simple Chinese Word Segmentation。是C语言开发的基于词频词典的机械式中文分词引擎。scws的作者为[hightman][1]，采用BSD许可协议发布。nodescws的作者在libscws上添加功能并添加了node.js binding，除自己的代码外，不持有任何libscws的著作权。

scws(Simple Chinese Word Segmentation) is a mechanistic Chinese word segement engine written in C. The author of this library is hightman. scws is published under BSD license. As the author of nodescws, I just added some features to the lib and wrap this great library as a node addon, thus holding no copyright of any of the library's code but my own work.

scws的主页: [http://www.xunsearch.com/scws][2], GitHub: [https://github.com/hightman/scws][3]


##### Performance

在 FreeBSD 6.2 系统，单核单 CPU 至强 3.0G 的服务器上，测试长度为 80,535 的文本。 用附带的命令行工具耗时将约 0.17 秒。
分词精度 95.60%，召回率 90.51% (F-1: 0.93)

On a server with a single core Xeon CPU and 3.0G memory running FreeBSD 6.2, Segmenting a 80,535 text using the cli tool based on this library took 0.17 seconds, with the accuracy of 95.60% and recall of 90.51%(F-1 0.93).

------

## nodescws
Current release: v0.2.1 (versions lower than v0.2.0 are no longer maintained. See Changelog)

- 项目主页: [https://github.com/dotSlashLu/nodescws][4]
- 使用问题，bug report: [https://github.com/dotSlashLu/nodescws/issues][5]

### Install
`npm install scws`

### Usage
    var Scws = require("scws");
    var scws = new Scws.init(settings);
    var results = scws.segment(text);
    scws.destroy(); // DO NOT forget this or your memory may be corrupted


#### new Scws.init(settings)
* settings: `Object`, 分词设置, 支持charset, dicts, rule, ignorePunct, multi, debug
    - charset: `String`, *Optional*

            采用的encoding，支持"utf8"，"gbk"， 默认值"utf8"

    - dicts: `String`, *Required*

            要采用的词典文件的filename，多个文件之间用':'分隔。
            支持xdb格式以及txt格式，自制词典请以".txt"作文件后缀。
            例如"./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb:./dicts/dict.test.txt"
            scws自带的xdb格式词典附在该extension目录下(一般是node_modules/scws/)的./dicts/ ，
            有简体和繁体两种选择，如果该项缺失则默认使用自带utf8简体中文词典

    - rule: `String`, *Optional*

            要采用的规则文件，设置对应编码下的地名，人名，停用词等。
            详见该extension目录下(一般是node_modules/scws/)的rules/rules.utf8.ini。
            若该配置缺失则默认使用自带utf8的规则文件。

      		v0.2.3添加了JSON支持，避免繁复的ini语法。
      		若以.json结尾，则会解析对应的JSON rule文件，也可以直接传JSON string来进行配置。


    - ignorePunct: `Bool`, *Optional*

            是否忽略标点

    - multi: `String`, *Optional*

            是否进行长词复合切分，例如中国人这个词产生“中国人”，“中国”，“人”多个结果，可选值"short", "duality", "zmain", "zall":
                short: 短词
                duality: 组合相邻的两个单字
                zmain: 重要单字
                zall: 全部单字

    - debug: `Bool`, *Optional*

            是否以debug模式运行，若为true则输出scws的log, warning, error到stdout, defult为false

    - applyStopWord: `Bool`, *Optional*

            是否应用rule文件中[nostats]区块所规定的停用词，默认为true

#### scws.segment(text)

* text: `String`, 要切分的字符串

Return `Array`

    [
        {
            word: '可读性',
            offset: 183, // 该词在文档中的位置
            length: 9, // byte
            attr: 'n', // 词性，采用《现代汉语语料库加工规范——词语切分与词性标注》标准，涵义请参考 http://blog.csdn.net/dbigbear/article/details/1488087
            idf: 7.800000190734863
        },
        ...
    ]

### Example 用例

    var fs   = require("fs")
        Scws = require("scws");

    fs.readFile("./test_doc.txt", {
      encoding: "utf8"
    }, function(err, data){
      if (err)
        return console.error(err);

      // initialize scws with config entries
      var scws = new Scws.init({
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
      res1 = scws.segment("大家好我来自德国，我是德国人");

      console.log(res);
      console.log("test reuse of scws: ", res1);

      // destroy scws, recollect memory
      scws.destroy();
    })

### Changelog
#### v0.2.3
- Changed project structure
- Refactored node bindings
- Added rule setting by JSON file and JSON string thus making adding stop words more easier with node

#### v0.2.2
- Some small bug fixes, including issue #5(Thanks to @Frully)

#### v0.2.1
- Add stop words support
- Remove line endings when `ignorePunct` is set true

You can add your own stop words in the entry `[nostats]` in the rule file. Turn off stop words feature by setting `applyStopWord` false.

#### v0.2.0
New syntax to initialize scws: `scws = new Scws(config); result = scws.segment(text); scws.destroy()` so that we are able to reuse scws instance, thus gaining great improvement in perfermence when recurrently used(approximately 1/4 faster).

Added new setting entry `debug`. Setting `config.debug = true` will make scws output it's log, error, warning to stdout

#### v0.1.3
Published to npm registry. usage: `scws(text, settings);` available setting entries: charset, dicts, rule, ignorePunct, multi.



[1]: http://www.hightman.cn
[2]: http://www.xunsearch.com/scws/
[3]: https://github.com/hightman/scws
[4]: https://github.com/dotSlashLu/nodescws
[5]: https://github.com/dotSlashLu/nodescws/issues

