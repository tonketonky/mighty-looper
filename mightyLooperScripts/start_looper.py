import os
import signal
import subprocess
import time
from bt_server import BtServer
from logger import *

bt_server = BtServer()
shutdown_flag = False

def main():

    def signal_handler(sig, frame):
        log(TAG_ML, MSG_SIGINT_RECEIVED)
        os.killpg(os.getpgid(pd_process.pid), signal.SIGINT)
        log(TAG_LOOPER_CORE, MSG_STOPPED)
        bt_server.join()
        global shutdown_flag
        shutdown_flag = True

    log(TAG_ML, MSG_RUNNING)

    # start bluetooth server
    bt_server.start()

    # wait until bluetooth server is initialized
    while not(bt_server.is_up):
        # do nothing
        pass

    # start looper core
    pd_process = subprocess.Popen('sudo pd -nogui -path /home/pi/mighty_looper/puredata/pd_externals/ -lib mighty_looper_lib -audiobuf 5 /home/pi/mighty_looper/puredata/mighty_looper.pd > /dev/null 2>&1', shell=True, preexec_fn=os.setpgrp)
    log(TAG_LOOPER_CORE, MSG_RUNNING)

    # register handler for interrupt signal
    signal.signal(signal.SIGINT, signal_handler)

    # keep it up until shutdown_flag is not set
    while not shutdown_flag:
        time.sleep(1)

    log(TAG_ML, MSG_BYE)

if __name__ == '__main__':
    main()

