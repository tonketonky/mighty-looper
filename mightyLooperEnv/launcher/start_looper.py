import bt_service
import signal
import os

def signal_handler(sig, frame):
        print('SIGINT received')
        bt_service.clean_up()
        sys.exit(0)

# register handler for interrupt signal
signal.signal(signal.SIGINT, signal_handler)

# start bluetooth service
bt_service.start()

# run Pure Data patch
os.system('sudo pd -nogui -path /home/pi/mighty_looper/puredata/pd_externals/ -lib mighty_looper_lib -audiobuf 5 /home/pi/mighty_looper/puredata/mighty_looper.pd')
