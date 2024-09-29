import os
import sys

sys.path.append(os.path.join(os.path.abspath("./"), "build\\src"))

os.add_dll_directory("D:\\worktools\\develop\\mingw64\\bin")
import interface

from PySide6.QtCore import QObject, Signal, Slot
from PySide6.QtWidgets import QApplication, QWidget, QPushButton, QLabel, QVBoxLayout


class MyWidget(QWidget):
    def __init__(self):
        super().__init__()
        self.label = QLabel("Hello, PySide6!")
        self.button = QPushButton("Click me")
        self.button.clicked.connect(self.on_button_clicked)
        layout = QVBoxLayout()
        layout.addWidget(self.label)
        layout.addWidget(self.button)
        self.setLayout(layout)
        self.resize(600, 400)

    @Slot()
    def on_button_clicked(self):
        self.label.setText("Button clicked!")


def testQt():
    app = QApplication([])
    widget = MyWidget()
    widget.show()
    app.exec()


def test():
    print("test")
    s = interface.IpcServer("test")
    while True:
        s.listening()
        if len(s.connections()):
            print(s.connections()[0].roles()[0].name)



def main():
    test()


if __name__ == "__main__":
    main()
