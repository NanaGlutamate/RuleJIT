import subprocess
import sys

import numpy as np
from matplotlib import pyplot as plt
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget


class Window(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setup_ui()

    def setup_ui(self):
        window = QtWidgets.QWidget()

        self.group_box = QtWidgets.QGroupBox('设置')

        self.start_gen_button = QtWidgets.QPushButton("开始代码生成")
        self.start_gen_button.clicked.connect(self.start)

        self.process_output = QtWidgets.QPlainTextEdit(self)
        self.process_output.setReadOnly(True)

        self.input_file_button = QtWidgets.QPushButton("选择输入XML文件")
        self.input_file_button.clicked.connect(self.ask_for_input_file)
        self.input_file = QtWidgets.QLineEdit(self)
        self.input_file.setReadOnly(True)

        self.output_path_button = QtWidgets.QPushButton("选择输出文件夹")
        self.output_path_button.clicked.connect(self.ask_for_output_dir)
        self.output_path = QtWidgets.QLineEdit(self)
        self.output_path.setText("./src/")

        self.prefix_text = QtWidgets.QLabel("设置输出文件前缀（可为空）")
        self.prefix = QtWidgets.QLineEdit(self)

        self.namespace_text = QtWidgets.QLabel("设置输出文件命名空间")
        self.namespace = QtWidgets.QLineEdit(self)
        self.namespace.setText("ruleset")

        settings_layout = QtWidgets.QGridLayout()
        settings_layout.addWidget(self.input_file_button, 0, 0)
        settings_layout.addWidget(self.input_file, 0, 1)
        settings_layout.addWidget(self.output_path_button, 1, 0)
        settings_layout.addWidget(self.output_path, 1, 1)
        settings_layout.addWidget(self.prefix_text, 2, 0)
        settings_layout.addWidget(self.prefix, 2, 1)
        settings_layout.addWidget(self.namespace_text, 3, 0)
        settings_layout.addWidget(self.namespace, 3, 1)
        self.group_box.setLayout(settings_layout)

        outer_layout = QtWidgets.QVBoxLayout()
        outer_layout.addWidget(self.group_box)
        outer_layout.addWidget(self.start_gen_button)
        text = QtWidgets.QLabel("代码生成工具输出")
        outer_layout.addWidget(text)
        outer_layout.addWidget(self.process_output)
        window.setLayout(outer_layout)

        # frame = QtWidgets.QWidget()
        self.setCentralWidget(window)

    def ask_for_output_dir(self):
        o = QtWidgets.QFileDialog.getExistingDirectory(self, "输出路径", ".")
        self.output_path.setText(str(o))

    def ask_for_input_file(self):
        f, _ = QtWidgets.QFileDialog.getOpenFileName(self, "输入文件", ".", "*.xml")
        self.input_file.setText(str(f))

    def start(self):
        i = self.input_file.text()
        o = self.output_path.text()
        p = self.prefix.text()
        n = self.namespace.text()
        if i == '' or o == '':
            self.process_output.setPlainText('必须设置输入文件与输出文件')
            return
        if n == '':
            self.process_output.setPlainText('命名空间不能为空')
            return
        cmd = f'./cq_codegen.exe {i} -o {o} -n {n}'
        if p != '':
            cmd += f' -p {p}'
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=False)
        output = ''
        while (ret := p.poll()) is None:
            line = p.stdout.readline()
            output += str(line, encoding='utf-8')
        if output == '':
            self.process_output.setPlainText('成功')
        else:
            self.process_output.setPlainText(output)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    widget = Window()
    widget.resize(640, 480)
    widget.setWindowTitle("codegen")
    widget.show()
    sys.exit(app.exec())