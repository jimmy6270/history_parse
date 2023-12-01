# history_parse
1. 需要进入history_parse目录运行程序
2. 程序运行方法是：./history_parse "20 ce 48 e2 67 65 10 01 67 00"；如果数据之间没有空格，也可以不加双引号
3. 不同产品数据格式不一样，需要修改根目录下的config.json，否则数据会解析异常
4. 如果不想看数据内容，只想解析时间，可以删掉config.json，或者将config.json里面的format_enable字段设置为0
