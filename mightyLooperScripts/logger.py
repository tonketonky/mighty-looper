import sys

# constants
TAG_BT_SERVER = 'bluetooth server'
TAG_LOOPER_CORE = 'looper core'
TAG_ML = 'ml'
TAG_SERIAL_BRIDGE = 'serial bridge'

MSG_ACCEPTED_CONNECTION_FROM = 'accepted connection from '
MSG_BYE = 'bye'
MSG_CLIENT_NOT_CONNECTED_DROPPING_SIGNAL_FROM_CORE = 'client not connected, dropping signal from core: '
MSG_CLIENT_SOCKET_CLOSET = 'client socket closed'
MSG_CORE_TO_GUI = 'core -> gui: '
MSG_DISCONNECTED = 'disconnected'
MSG_GUI_TO_CORE = 'gui -> core: '
MSG_LISTENING_FOR_CONNECTION = 'listening for connection...'
MSG_LISTENING_FOR_DATA_FROM_CORE = 'listening for data from core...'
MSG_LISTENING_FOR_DATA_FROM_GUI = 'listening for data from gui...'
MSG_MASTER_CLOSED = 'master closed'
MSG_PSEUDO_TERMINALS_CREATED = 'pseudo terminals created'
MSG_PSEUDO_TERMINALS_DESTROYED = 'pseudo terminals destroyed'
MSG_RUNNING = 'running...'
MSG_SERVER_SOCKET_CLOSED = 'server socket closed'
MSG_SIGINT_RECEIVED = 'SIGINT received'
MSG_SLAVE_CLOSED = 'slave closed'
MSG_STOPPED = 'stopped'

# log to console
def log(module, message):
    print('[{}] {}'.format(module, message))
    sys.stdout.flush()