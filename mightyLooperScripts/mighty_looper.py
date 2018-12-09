import os
import signal
import subprocess
import time
from bt_server import BtServer
from logger import *

LOOPER_HOME_ENV_VAR = 'LOOPER_HOME'

bt_server = BtServer()
shutdown_flag = False

def main():

    def signal_handler(sig, frame):
        if sig == 2:
            log(TAG_ML, MSG_SIGINT_RECEIVED)
        else:
            log(TAG_ML, MSG_SIGTERM_RECEIVED)
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
    looper_home = os.getenv(LOOPER_HOME_ENV_VAR)
    pd_process = subprocess.Popen('sudo pd -nogui -path /home/pi/mighty_looper/puredata/pd_externals/ -lib mighty_looper_lib -audiobuf 5 /home/pi/mighty_looper/puredata/mighty_looper.pd 2>>{}/log/mighty-looper-core-console.log'.format(looper_home, looper_home), shell=True, preexec_fn=os.setpgrp)
    log(TAG_LOOPER_CORE, MSG_RUNNING)

    # register handler for interrupt signal
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # keep it up until shutdown_flag is not set
    while not shutdown_flag:
        time.sleep(1)

    log(TAG_ML, MSG_BYE)

if __name__ == '__main__':
    main()

