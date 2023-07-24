import traceback
from PyQt5 import QtWidgets, uic
from PyQt5.QtSerialPort import QSerialPort, QSerialPortInfo
from PyQt5.QtCore import QIODevice, QDateTime
from PyQt5.QtWidgets import QApplication, QMessageBox, QListWidgetItem
import sqlite3
import win32print
import os
from PyQt5.QtGui import QStandardItem, QStandardItemModel

app = QtWidgets.QApplication([])
ui = uic.loadUi("design.ui")
ui.setWindowTitle("Sverimas")
portList = []
ports = QSerialPortInfo.availablePorts()
for port in ports:
    portList.append(port.portName())
print(portList)
ui.comL.addItems(portList)


# Create a connection to the SQLite database
conn = sqlite3.connect("weight_data.db")
c = conn.cursor()

# Create a table for weight data if it doesn't exist
c.execute(
    """
    CREATE TABLE IF NOT EXISTS weight (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        mass REAL,
        datetime DATETIME,
        shift INTEGER
    )
    """
)

# Create the directory for CSV files if it doesn't exist
csv_dir = "C://Packing"
if not os.path.exists(csv_dir):
    os.makedirs(csv_dir)

list_model = QStandardItemModel()
ui.list_view.setModel(list_model)

serial = QSerialPort()
serial.setBaudRate(115200)


def onRead():
    if not serial.canReadLine():
        return

    rx = serial.readLine()
    rxs = str(rx, 'utf-8')
    print(rxs)

    # Remove leading/trailing whitespaces, curly braces, and split the string into individual values
    rxs = rxs.strip().strip("{}").split(", ")

    # Convert each value to the appropriate data type
    mass = float(rxs[0])
    saveB = int(rxs[1])
    zeroB = int(rxs[2])
    blueLedPin = int(rxs[3])
    greenLedPin = int(rxs[4])
    redLedPin = int(rxs[5])

    print(mass, saveB, zeroB, blueLedPin, greenLedPin, redLedPin)
    led_indication(blueLedPin, greenLedPin, redLedPin, mass, saveB)

def onOpen():
    serial.setPortName(ui.comL.currentText())
    serial.open(QIODevice.ReadWrite)

def onClose():
    serial.close()
    ui.saveB.setEnabled(False)

def led_indication(blueLedPin, greenLedPin, redLedPin, mass, saveB):
    ui.blueLed.setStyleSheet("background-color: blue" if blueLedPin == 1 else "")
    ui.greenLed.setStyleSheet("background-color: green" if greenLedPin == 1 else "")
    ui.redLed.setStyleSheet("background-color: red" if redLedPin == 1 else "")
    ui.mass.setText(str(mass))
    if saveB == 1:
        save_weight()

def print_text(text):
    # Get the default printer
    default_printer = win32print.GetDefaultPrinter()

    # Create a printer handle
    printer_handle = win32print.OpenPrinter(default_printer)

    try:
        # Start a print job
        job_info = win32print.StartDocPrinter(printer_handle, 1, ("Print Job", None, "RAW"))

        try:
            # Write the text to the printer
            win32print.WritePrinter(printer_handle, text.encode('utf-8'))

        finally:
            # End the print job
            win32print.EndDocPrinter(printer_handle)

    finally:
        # Close the printer handle
        win32print.ClosePrinter(printer_handle)


data_list = []  # Create an empty list to store the data


def update_list_widget():
    list_model.clear()  # Clear the model before updating it
    for data in data_list:
        item = QStandardItem(data)
        list_model.appendRow(item)


def save_weight():
    if serial.isOpen():
        mass = int(ui.mass.text())

        # Get the selected shift value
        if ui.shift_1.isChecked():
            shift = 1
        elif ui.shift_2.isChecked():
            shift = 2
        elif ui.shift_3.isChecked():
            shift = 3

        current_datetime = QDateTime.currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
        print()
        try:
            # Insert weight data into the SQLite database
            c.execute(
                """
                INSERT INTO weight (mass, datetime, shift) VALUES (?, ?, ?)
                """,
                (float(mass), current_datetime, shift),
            )
            conn.commit()

            # Change save button color to green
            ui.saveB.setStyleSheet("background-color: green;")

            # Check if the "Print Output" checkbox is checked
            if ui.printoutas.isChecked():
                # Print the text
                # print_text(item_text)
                pass

            # Update the data_list with the new data
            data_list.append(f"Shift: {shift}, Mass: {mass} g, Datetime: {current_datetime}")
            print(data_list)

            # Update the list view with the new data
            update_list_widget()

        except Exception as e:
            print(f"An error occurred: {e}")
            traceback.print_exc()

    else:
        pass
        # show_comport_popup()


def serial_send_mass_set():   # integer string
    txs = "2,"
    txs += str(int(ui.mass_set.text())) + ";"
    print(txs)
    serial.write(txs.encode())

def serial_send_accuracy():  # integer string
    txs = "3," + str(ui.accuracy.text()) + ";"  # Multiply by 10 to send the value as an integer (e.g., 123.4 becomes 1234)
    print(txs)
    serial.write(txs.encode())

def serial_send_zeroB():  # integer string
    txs = "0,1;"
    print(txs)
    serial.write(txs.encode())

def serial_send_accuracy_minus(state):  # integer string
    if state == 2:
        state = 1
    txs = "1," + str(int(state)) + ";"
    print(txs)
    serial.write(txs.encode())


ui.send_accuracy_minus_button.stateChanged.connect(serial_send_accuracy_minus)
serial.readyRead.connect(onRead)
ui.openB.clicked.connect(onOpen)
ui.closeB.clicked.connect(onClose)
ui.send_mass_set_button.clicked.connect(serial_send_mass_set)
ui.accuracy_button.clicked.connect(serial_send_accuracy)
ui.zero.clicked.connect(serial_send_zeroB)
ui.send_accuracy_minus_button.stateChanged.connect(serial_send_accuracy_minus)

ui.show()
app.exec()
