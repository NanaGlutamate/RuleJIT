# 整体结构

项目整体是一系列规则建模的工具，包含可以接入cqsim的解释器动态库主体(cq_interpreter.dll)，代码生成工具(cq_codegen.exe)，模型描述文件生成工具(cq_modelxmlgen.exe)与原定接入规则编辑工具的语法检查工具(cq_expressionchecker.dll)

这些工具均以XML格式的规则行为模型描述文件(例子:`doc/xml_design/example1.0.xml`)作为输入

## 1. cq_interpreter.dll

该解释器可以直接作为行为模型的*模型动态库*录入cqsim，默认以*模型依赖项*中的"rule.xml"作为规则集描述文件；同时支持名为"filePath"的初始化参数，可以通过以绝对路径指定该参数以避免修改xml后重新上传*模型依赖项*

## 2. cq_codegen.exe

通过描述文件XML生成行为模型动态库的CMake工程，build之后可以直接上传至cqsim替换解释器dll

在控制台直接输入`.\cq_codegen.exe`可以查看使用方法

## 3. cq_modelxmlgen.exe

通过规则行为模型描述文件生成cqsim要求的模型描述文件，避免重复编写

在控制台直接输入`.\cq_modelxmlgen.exe`可以查看使用方法

## 4. cq_expressionchecker.dll

目前未使用，可用于在给定上下文环境中检查表达式语法

# XML规则行为模型描述文件语法

支持的XML格式已在`example1.0.xsd`中定义，手工编写XML时只需将该xsd文件放入xml所在目录，并通过安装了XML扩展的VSCode打开xml即可使用格式检查与代码提示功能

# 表达式语法

XML中的Expression标签中的文本均为表达式，以使用户能够自定义代码逻辑；表达式支持的语法如下

## 变量定义

## 控制流
