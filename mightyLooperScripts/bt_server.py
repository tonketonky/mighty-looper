import bluetooth
import os
import serial
import signal
import subprocess
import threading
from pathlib import Path

uuid = '1e0ca4ea-299d-4335-93eb-27fcfe7fa848'
master_link_path = '/tmp/mlbtmaster'
slave_link_path = '/tmp/mlbtslave'

class BtServer(threading.Thread):
    'Bluetooth server for connection with MightyLooper GUI Android app'

    def __init__(self):
        super(BtServer, self).__init__()
        self.shutdown_flag = threading.Event()

    def run(self):
        self.server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        self.server_sock.bind(("",0))
        self.server_sock.settimeout(1)
        self.server_sock.listen(1)
        self.serial_bridge = SerialBridge(self)
        self.serial_bridge.start();
        bluetooth.advertise_service( self.server_sock, "Mighty Looper BT Service", service_id = uuid, profiles = [ bluetooth.SERIAL_PORT_PROFILE ] )
        self.listen()

    def send(self, data):
        self.client_sock.send(data)


    def listen(self):
        self.connected = False
        print ('Listening for connection')
        while not self.shutdown_flag.is_set():
            if self.connected:
                # listen for data
                try:
                    data = self.client_sock.recv(1024)
                except bluetooth.btcommon.BluetoothError as e:
                    if e.args[0] == 'timed out':
                        # read timeout
                        continue
                    elif e.args[0] == '(104, \'Connection reset by peer\')':
                        # lost connection
                        self.connected = False
                        print ('Disconnected')
                        print ('Listening for connection')
                        continue
                    else:
                        raise e
                # data received, write to serial bridge
                self.serial_bridge.write(data)
            else:
                # listen for connection
                try:
                    self.client_sock, self.address = self.server_sock.accept()
                    self.client_sock.settimeout(1)
                    self.connected = True
                    print ('Accepted connection from ', self.address)
                    print ('Listening for data')
                except bluetooth.btcommon.BluetoothError:
                    # connection timeout
                    continue

    def join(self, timeout=None):
        self.shutdown_flag.set()
        self.server_sock.close()
        try:
            self.client_sock.close()
        except AttributeError:
            pass
        self.serial_bridge.join()
        super(BtServer, self).join(timeout)
        print("Bluetooth server stopped")

class SerialBridge(threading.Thread):
    'Bridge between bluetooth server and mighty_looper.pd Pure Data patch which uses serial port for communication'
    def __init__(self, bt_server):
        super(SerialBridge, self).__init__()
        self.bt_server = bt_server
        self.shutdown_flag = threading.Event()

    def run(self):
        # create pseudo-terminals in new thread
        self.create_pseudo_terminals()

        # wait until pseudo-terminals with links are created
        while not(Path(master_link_path).is_symlink()):
            # do nothing
            pass

        print ('Pseudo terminals created')
        # open serials
        self.master = serial.Serial(master_link_path, timeout=1)

        # listen for data from slave
        self.listen_for_data();
    def create_pseudo_terminals(self):
        self.terminals_thread = subprocess.Popen('socat -d -d pty,link={},raw,echo=0 pty,link={},raw,echo=0 > /dev/null 2>&1'.format(master_link_path, slave_link_path), shell=True, preexec_fn=os.setpgrp)

    def listen_for_data(self):
        print('Listening for data from slave...')
        while not self.shutdown_flag.is_set():
            data = self.master.readline().decode("utf-8").strip()
            if data != "":
                self.bt_server.send(data)

    def write(self, data):
        self.master.write(data)

    def join(self, timeout=None):
        self.shutdown_flag.set()
        try:
            self.master.close()
        except AttributeError:
            pass
        os.killpg(os.getpgid(self.terminals_thread.pid), signal.SIGINT)
        print("Pseudo terminals destroyed")
        super(SerialBridge, self).join(timeout)
        print("Serial bridge stopped")


