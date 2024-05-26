整体行为

1. 表达式新增语法：
    1. 成员访问：类似C语言，使用.访问
    2. 数组元素访问：类似C语言，使用[]访问
    3. 条件表达式：if(条件)真值返回值 else 假值返回值
2. 描述文件新增元素：
    1. TypeDefines，格式类似生成的模型模板中的typedefines，用于解析复杂类型
    2. Cache，即缓存变量，其行为是上一帧向缓存变量写入的值可以用于下一帧的输入
    3. Cache和Output的Value节点，其中的表达式将在每次Tick开始计算并赋值给相应对象
    4. 不同子规则集不可以同时写入同一个缓存变量或输出变量，对复杂类型的缓存、输出变量的不同成员的写入也视为同时写入

2023-04修改

1. 数组操作：
   1. `array.length()` 用于获取数组长度
2. 原子规则后件：
   1. 新增后件类型 `resize` 用于调整数组大小，l是大于等于0的任意大小
   2. 新增后件类型 `push` 用于在数组尾部插入新元素
   3. 现在支持后件中的多个操作；这些操作是顺序执行的
3. 中间变量支持：
   1. 在输入/输出/缓存变量的定义中添加Value节点使其成为中间变量，中间变量可以有依赖关系但不能有循环依赖。推理机会自动根据依赖关系对赋值操作进行拓扑排序

2024-03修改

1. 自动收集数组：当Inputs中声明类型为数组而传入的实际类型为数组元素类型时，自动收集元素到数组中并在所有input完成后单独input自动收集的数组以覆盖错误值
   *注意*：目前仅解释器支持该功能
2. 初始化：目前会将所有初始化参数保存到规则集的输入中

2024-05修改

1. 代码生成工具也支持自动数组收集了

源码结构

1. XML文件格式要求：在doc/xml_design/example1.0.xml中，同文件夹的example1.0.xsd是其语法约束文件
2. 解析工具源码：在src文件夹下，可通过${FRONTEND_SRC}和${BACKEND_SRC}两个CMake变量访问；解析工具的测试源码在src/test/cqmain.cpp，对应的CMake目标为cq_test.e
3. 解析dll源码：主要文件在src/release/cq_interpreter下，同时包含test_main