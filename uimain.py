import os
import subprocess
import sys
import threading

import numpy as np
from matplotlib import pyplot as plt
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget
from PyQt5.QtCore import pyqtSignal


class Window(QMainWindow):
    set_output = pyqtSignal(str)

    def __init__(self) -> None:
        super().__init__()
        self.set_output.connect(lambda s:self.process_output.setPlainText(s))
        self.setup_ui()

    def setup_ui(self):
        window = QtWidgets.QWidget()

        self.group_box = QtWidgets.QGroupBox("设置")

        self.process_output = QtWidgets.QPlainTextEdit(self)
        self.process_output.setReadOnly(True)

        self.input_file_button = QtWidgets.QPushButton("选择输入XML文件")
        self.input_file_button.clicked.connect(self.ask_for_input_file)
        self.input_file = QtWidgets.QLineEdit(self)
        self.input_file.setReadOnly(True)

        self.output_path_button = QtWidgets.QPushButton("选择输出文件夹")
        self.output_path_button.clicked.connect(self.ask_for_output_dir)
        self.output_path = QtWidgets.QLineEdit(self)

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

        start_gen_button = QtWidgets.QPushButton("开始代码生成")
        start_gen_button.clicked.connect(self.start)
        outer_layout.addWidget(start_gen_button)

        build_button = QtWidgets.QPushButton("构建动态库")
        build_button.clicked.connect(self.build)
        outer_layout.addWidget(build_button)

        outer_layout.addWidget(QtWidgets.QLabel("执行命令"))

        self.running_command = QtWidgets.QLineEdit(self)
        self.running_command.setReadOnly(True)
        outer_layout.addWidget(self.running_command)
        
        outer_layout.addWidget(QtWidgets.QLabel("输出"))

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

    def run_command(self, cmd, callback=None):
        self.running_command.setText(str(cmd))
        def func():
            output = ''
            p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=False)
            while (ret := p.poll()) is None:
                line = p.stdout.readline()
                output += line.decode(encoding='utf-8', errors='ignore')
                self.set_output.emit(output)
            if output == '':
                self.set_output.emit('成功')
            if callback is not None:
                callback()
        threading.Thread(target=func).start()

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
        self.run_command(cmd)

    def build(self):
        o = self.output_path.text()
        if o == '':
            self.process_output.setPlainText('必须设置输出文件')
            return
        try:
            os.mkdir(f'{o}/build')
        except FileExistsError as e:
            pass
        os.chdir(f'{o}/build')
        def func():
            self.run_command(f'cmake --build . --config RelWithDebInfo')
        self.run_command(f'cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo', func)


if __name__ == '__main__':
    app = QApplication(sys.argv)
    widget = Window()
    widget.resize(640, 480)
    widget.setWindowTitle("codegen")
    widget.show()
    sys.exit(app.exec())