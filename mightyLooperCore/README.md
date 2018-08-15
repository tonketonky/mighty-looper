# Mighty Looper core

All the looper logic is implemented here in libraries located in pd_externals. Current state of looper is held inside of them
and signals with appropriate destination are sent to to Pure Data patch which connects all controlling components and routes
signals between them.

Running module:

1. build libraries from sources:
```
$ cd mightyLooperCore/pd_externals/sources
$ make clean
$ make install
```

`mighty_looper_lib` will be created in `mightyLooperCore/pd_externals`

NOTE: It matters where libraries are built, if the module is to be run on RPi, build them on RPi.

2. run pd patch:
```
$ sudo pd -nogui -path PATH-TO-MIGHTY-LOOPER-CORE-FOLDER/pd_externals/ -lib mighty_looper_lib -audiobuf 5 PATH-TO-MIGHTY-LOOPER-CORE-FOLDER/mighty_looper.pd
```
