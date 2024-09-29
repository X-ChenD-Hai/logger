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
        self.resize(600,400)

    @Slot()
    def on_button_clicked(self):
        self.label.setText("Button clicked!")

if __name__ == "__main__":
    app = QApplication([])
    widget = MyWidget()
    widget.show()
    app.exec()