# nodescws
----------

## scws

#### 关于 About
scws即Simple Chinese Word Segmentation。是C语言开发的基于词频词典的机械式中文分词引擎。scws的作者为[hightman][1]，采用BSD许可协议发布。nodescws的作者仅是将这个伟大的library包装成为node.js addon，除自己的代码外，不持有任何scws库的著作权。

scws(Simple Chinese Word Segmentation) is a mechanistic Chinese word segement engine written in C. The author of this library is hightman. scws is published under BSD license. As the author of nodescws, I just wrap this great library as a node addon, thus holding no copyright of any of the library's code but my own work.

scws的主页: [http://www.xunsearch.com/scws][2]

scws的GitHub: [https://github.com/hightman/scws][3]


#### 性能指标 Performance

在 FreeBSD 6.2 系统，单核单 CPU 至强 3.0G 的服务器上，测试长度为 80,535 的文本。 用附带的命令行工具耗时将约 0.17 秒，若改用 php 扩展方式调用，则耗时约为 0.65 秒。
分词精度 95.60%，召回率 90.51% (F-1: 0.93)

On a server with a single core Xeon CPU and 3.0G memory running FreeBSD 6.2, Segmenting a 80,535 text using the cli tool based on this library took 0.17 seconds, while using php extension, it's 0.65 seconds, with the accuracy of 95.60% and recall of 90.51%(F-1 0.93).

------

## nodescws
### Install
`node-gyp rebuild` or `npm install`

### Usage
`scws.segment(text, settings);`

Parameters:

* text: String, 要切分的字符串
* settings: Object, 分词设置, 支持charset, dicts, rule, ignorePunct, multi

Settings:

    charset: String, Optional
        采用的encoding，支持"utf8"，"gbk"， 默认值"gbk"

    dicts: String, Required
        要采用的词典文件的filename，多个文件之间用':'分隔。支持xdb格式以及txt格式，自制词典请以".txt"作文件后缀。例如“./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb:./dicts/dict.test.txt"
        
    rule: String, Optional
        要采用的规则文件，详见rules/rules.ini
        
    ignorePunct: Bool, Optional
        是否忽略标点
        
    multi: String, Optional
        是否进行长词复合切分，例如中国人这个词产生“中国人”，“中国”，“人”多个结果，可选值"short", "duality", "zmain", "zall"
        short: 短词
        duality: 组合相邻的两个单字
        zmain: 重要单字
        zall: 全部单字
        
Return: 

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

### Example

    var scws = require("scws"); // import the module
    var result,
        text = "中文字段",
        settings = {
            charset: "utf8",
            dicts: "./dicts/dict.utf8.xdb:./dicts/dict_cht.utf8.xdb:./dicts/dict.test.txt",
            rule: "./rules/rules.utf8.ini",
            ignorePunct: true,
            multi: "duality"
        };
    result = scws.segment(text, settings);
    
    
[1]: http://www.hightman.cn
[2]: http://www.xunsearch.com/scws/
[3]: https://github.com/hightman/scws
