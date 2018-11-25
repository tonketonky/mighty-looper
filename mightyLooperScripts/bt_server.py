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
        self.is_up = False

    def run(self):
        self.server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        self.server_sock.bind(("",0))
        self.server_sock.settimeout(1)
        self.server_sock.listen(1)
        self.serial_bridge = SerialBridge(self)
        self.serial_bridge.start();
        bluetooth.advertise_service( self.server_sock, "Mighty Looper BT Service", service_id = uuid, profiles = [ bluetooth.SERIAL_PORT_PROFILE ] )
        self.is_up = True
        print('[bluetooth server] running...')
        self.listen()

    def listen(self):
        self.connected = False
        print('[bluetooth server] listening for connection...')
        while not self.shutdown_flag.is_set():
            if self.connected:
                # listen for data from gui
                try:
                    data = self.client_sock.recv(1024)
                except bluetooth.btcommon.BluetoothError as e:
                    if e.args[0] == 'timed out':
                        # read timeout
                        continue
                    elif e.args[0] == '(104, \'Connection reset by peer\')':
                        # lost connection
                        self.connected = False
                        print('[bluetooth server] disconnected')
                        print('[bluetooth server] listening for connection...')
                        continue
                    else:
                        raise e
                # data received, write to serial bridge
                print('[bluetooth server] gui -> core: ' + str(data))
                self.serial_bridge.write(data)
            else:
                # listen for connection
                try:
                    self.client_sock, self.address = self.server_sock.accept()
                    self.client_sock.settimeout(1)
                    self.connected = True
                    print('[bluetooth server] accepted connection from ', self.address)
                    print('[bluetooth server] listening for data from gui...')
                except bluetooth.btcommon.BluetoothError:
                    # connection timeout
                    continue

    def send(self, data):
        print('[bluetooth server] core -> gui: ' + str(data))
        self.client_sock.send(data)

    def join(self, timeout=None):
        self.shutdown_flag.set()
        try:
            self.server_sock.close()
            print('[bluetooth server] server socket closed')
        except AttributeError:
            pass
        try:
            self.client_sock.close()
            print('[bluetooth server] client socket closed')
        except AttributeError:
            pass
        self.serial_bridge.join()
        super(BtServer, self).join(timeout)
        print('[bluetooth server] stopped')

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

        print('[serial bridge] pseudo terminals created')
        # open serials
        self.master = serial.Serial(master_link_path, timeout=1)
        self.slave = serial.Serial(slave_link_path)
        # listen for data from slave
        self.listen();

    def create_pseudo_terminals(self):
        self.terminals_thread = subprocess.Popen('socat -d -d pty,link={},raw,echo=0 pty,link={},raw,echo=0 > /dev/null 2>&1'.format(master_link_path, slave_link_path), shell=True, preexec_fn=os.setpgrp)

    def listen(self):
        print('[serial bridge] listening for data from core...')
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
            print('[serial bridge] master closed')
        except AttributeError:
            pass
        try:
            self.slave.close()
            print('[serial bridge] slave closed')
        except AttributeError:
            pass
        os.killpg(os.getpgid(self.terminals_thread.pid), signal.SIGINT)
        print("[serial bridge] pseudo terminals destroyed")
        super(SerialBridge, self).join(timeout)
        print("[serial bridge] stopped")


