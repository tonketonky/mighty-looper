import bluetooth
import os
import serial
import threading
from pathlib import Path

class BtServer:
    'Bluetooth server for connection with MightyLooper GUI Android app'
    def __init__(self):
        self.uuid = '1e0ca4ea-299d-4335-93eb-27fcfe7fa848'
        self.server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        self.server_sock.bind(("",0))
        self.server_sock .listen(1)
        self.serial_bridge = SerialBridge(self)
        bluetooth.advertise_service( self.server_sock, "Mighty Looper BT Service", service_id = self.uuid, profiles = [ bluetooth.SERIAL_PORT_PROFILE ] )
        self.listen_for_connection()

    def send(self, data):
        self.client_sock.send(data)

    def listen_for_connection(self):
        print ('Listening for connection...')
        self.client_sock, self.address = self.server_sock.accept()
        print ('Accepted connection from ', self.address)

        # listen for data from gui
        listening_thread = threading.Thread(target = self.listen_for_data, args = ())
        listening_thread.start()

    def listen_for_data(self):
        print ('Listening for data...')
        while True:
            data = self.client_sock.recv(1024)
            self.serial_bridge.write(data)

    def clean_up(self):
        client_sock.close()
        server_sock.close()
        self.serial_bridge.clean_up()

class SerialBridge:
    'Bridge between bluetooth server and mighty_looper.pd Pure Data patch which uses serial port for communication'
    def __init__(self, bt_server):
        self.bt_server = bt_server
        self.master_link_path = '/tmp/mlbtmaster'
        self.slave_link_path = '/tmp/mlbtslave'

        # create pseudo-terminals in new thread
        terminals_thread = threading.Thread(target = self.create_pseudo_terminals, args = ())
        terminals_thread.start()

        # wait until pseudo-terminals with links are created
        while not(Path(self.master_link_path).is_symlink()):
            # do nothing
            pass

        print ('Pseudo terminals created')
        # open serials
        self.master = serial.Serial(self.master_link_path)

        # listen for data from slave
        listening_thread = threading.Thread(target = self.listen_for_data, args = ())
        listening_thread.start()

    def create_pseudo_terminals(self):
        os.system('socat -d -d pty,link={},raw,echo=0 pty,link={},raw,echo=0'.format(self.master_link_path, self.slave_link_path))

    def listen_for_data(self):
        print('Listening for data from slave...')
        while True:
            data = self.master.readline().decode("utf-8").strip()
            self.bt_server.send(data)

    def write(self, data):
        self.master.write(data)

    def clean_up(self):
        self.master.close()

def start():
    bluetooth_server = BtServer()

def clean_up():
    bluetooth_server.clean_up()
