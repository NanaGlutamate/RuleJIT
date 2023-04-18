整体行为

1. 表达式新增语法：
    1. 成员访问：类似C语言，使用.访问
    2. 数组元素访问：类似C语言，使用[]访问
    3. 条件表达式：if(条件)真值返回值 else 假值返回值
2. 描述文件新增元素：
    1. TypeDefines，格式类似生成的模型模板中的typedefines，用于解析复杂类型
    2. Cache，即缓存变量，其行为是上一帧向缓存变量写入的值可以用于下一帧的输入
    3. Cache和Output的Value节点，其中的表达式将在每次Tick开始计算并赋值给相应对象；若表达式中含有包含Value节点的Cache和Output变量，计算顺序是没有保证的

源码结构

1. XML文件格式要求：在doc/xml_design/example1.0.xml中，同文件夹的example1.0.xsd是其语法约束文件
2. 解析工具源码：在src文件夹下，可通过${FRONTEND_SRC}和${BACKEND_SRC}两个CMake变量访问；解析工具的测试源码在src/test/cqmain.cpp，对应的CMake目标为cq_test.e
3. 解析dll源码：主要文件在src/release/cq_interpreter下