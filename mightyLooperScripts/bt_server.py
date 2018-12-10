import bluetooth
import os
import serial
import signal
import subprocess
import threading
from logger import *
from pathlib import Path

MASTER_LINK_PATH = '/tmp/mlbtmaster'
SLAVE_LINK_PATH = '/tmp/mlbtslave'

SPP_UUID = '1e0ca4ea-299d-4335-93eb-27fcfe7fa848'

BT_SERVICE_NAME = 'Mighty Looper BT Service'

SCRIPTS_DIR_ENV_VAR = 'SCRIPTS_DIR'
PROFILE_ENV_VAR = 'PROFILE'

CMD_SHUTDOWN = '[cmd/shutdown]'

class BtServer(threading.Thread):
    'Bluetooth server for connection with MightyLooper GUI Android app'

    def __init__(self):
        super(BtServer, self).__init__()
        self.shutdown_flag = threading.Event()
        self.is_up = False
        self.is_connected = False

    def run(self):
        self.server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )
        self.server_sock.bind(("",0))
        self.server_sock.settimeout(1)
        self.server_sock.listen(1)
        self.serial_bridge = SerialBridge(self)
        self.serial_bridge.start();
        bluetooth.advertise_service( self.server_sock, BT_SERVICE_NAME, service_id = SPP_UUID, profiles = [ bluetooth.SERIAL_PORT_PROFILE ] )
        self.is_up = True
        log(TAG_BT_SERVER, MSG_RUNNING)
        self.listen()

    def listen(self):
        log(TAG_BT_SERVER, MSG_LISTENING_FOR_CONNECTION)
        while not self.shutdown_flag.is_set():
            if self.is_connected:
                # listen for data from gui
                try:
                    data = self.client_sock.recv(1024)
                except bluetooth.btcommon.BluetoothError as e:
                    if e.args[0] == 'timed out':
                        # read timeout
                        continue
                    elif e.args[0] == '(104, \'Connection reset by peer\')':
                        # lost connection
                        self.is_connected = False
                        log(TAG_BT_SERVER, MSG_DISCONNECTED)
                        log(TAG_BT_SERVER, MSG_LISTENING_FOR_CONNECTION)
                        continue
                    else:
                        raise e
                # data received
                if data.decode('utf-8').strip() == CMD_SHUTDOWN:
                    # is terminate command
                    if os.getenv(PROFILE_ENV_VAR) == 'dev':
                        # running in dev profile, just log requested shutdown and set shutdown_opt to empty string to avoid shutting down
                        log(TAG_BT_SERVER, MSG_SHUTDOWN_COMMAND_RECEIVED_DEV_PROF)
                        shutdown_opt = ''
                    else:
                        # running in prod profile, set shutdown_opt to  '--shutdown'
                        log(TAG_BT_SERVER, MSG_SHUTDOWN_COMMAND_RECEIVED_PROD_PROF)
                        shutdown_opt = '--shutdown'
                    # execute stop-looper.sh
                    os.system('{}/stop-looper.sh {} > /dev/null'.format(os.getenv(SCRIPTS_DIR_ENV_VAR), shutdown_opt))
                else:
                    # is NOT terminate command, write data to serial bridge
                    log(TAG_BT_SERVER, MSG_GUI_TO_CORE + data.decode('utf-8'))
                    self.serial_bridge.write(data)
            else:
                # listen for connection
                try:
                    self.client_sock, self.address = self.server_sock.accept()
                    self.client_sock.settimeout(1)
                    self.is_connected = True
                    log(TAG_BT_SERVER, MSG_ACCEPTED_CONNECTION_FROM + str(self.address))
                    log(TAG_BT_SERVER, MSG_LISTENING_FOR_DATA_FROM_GUI)
                except bluetooth.btcommon.BluetoothError:
                    # connection timeout
                    continue

    def send(self, data):
        if self.is_connected:
            log(TAG_BT_SERVER, MSG_CORE_TO_GUI + str(data))
            self.client_sock.send(data)
        else:
            log(TAG_BT_SERVER, MSG_CLIENT_NOT_CONNECTED_DROPPING_SIGNAL_FROM_CORE + str(data))

    def join(self, timeout=None):
        self.shutdown_flag.set()
        try:
            self.client_sock.close()
            log(TAG_BT_SERVER, MSG_CLIENT_SOCKET_CLOSET)
        except AttributeError:
            pass
        try:
            self.server_sock.close()
            log(TAG_BT_SERVER, MSG_SERVER_SOCKET_CLOSED)
        except AttributeError:
            pass
        self.serial_bridge.join()
        super(BtServer, self).join(timeout)
        log(TAG_BT_SERVER, MSG_STOPPED)

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
        while not(Path(MASTER_LINK_PATH).is_symlink()):
            # do nothing
            pass

        log(TAG_SERIAL_BRIDGE, MSG_PSEUDO_TERMINALS_CREATED)
        # open serials
        self.master = serial.Serial(MASTER_LINK_PATH, timeout=1)
        self.slave = serial.Serial(SLAVE_LINK_PATH)
        # listen for data from slave
        self.listen();

    def create_pseudo_terminals(self):
        self.terminals_thread = subprocess.Popen('socat -d -d pty,link={},raw,echo=0 pty,link={},raw,echo=0 > /dev/null 2>&1'.format(MASTER_LINK_PATH, SLAVE_LINK_PATH), shell=True, preexec_fn=os.setpgrp)

    def listen(self):
        log(TAG_SERIAL_BRIDGE, MSG_LISTENING_FOR_DATA_FROM_CORE)
        while not self.shutdown_flag.is_set():
            data = self.master.readline().decode("utf-8").strip()
            if data != "" and not self.shutdown_flag.is_set():
                self.bt_server.send(data)

    def write(self, data):
        self.master.write(data)

    def join(self, timeout=None):
        self.shutdown_flag.set()
        try:
            self.master.close()
            log(TAG_SERIAL_BRIDGE, MSG_MASTER_CLOSED)
        except AttributeError:
            pass
        try:
            self.slave.close()
            log(TAG_SERIAL_BRIDGE, MSG_SLAVE_CLOSED)
        except AttributeError:
            pass
        os.killpg(os.getpgid(self.terminals_thread.pid), signal.SIGINT)
        log(TAG_SERIAL_BRIDGE, MSG_PSEUDO_TERMINALS_DESTROYED)
        super(SerialBridge, self).join(timeout)
        log(TAG_SERIAL_BRIDGE, MSG_STOPPED)
