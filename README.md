# Mighty Looper

Mighty Looper project is an attempt to build an open source, portable, low-cost, 2-channel, multi-track audio looping machine that allows to operate
within 2 individual loop phrases (verse/chorus). It is implemented in Pure Data extended by C libraries, runs on
Raspberry Pi and is controlled by Arduino based controls. The looper is supposed to be used in a single person performance with
multiple instruments. 

The project is currently still under development.

## What it does (will do) 

Here are main features and functionalities of the looper:

- 2 separate phrases (verse/chorus) possible to switch between while each of them:
	- consists of 8 tracks in 2 channels (4 tracks per channel)
	- is possible to individually reset and start over
- 2 separate channels each possible to: 
	- adjust volume
	- mute/unmute
	- switch bypass ON/OFF
- 4 separate tracks per channel each capable of
	- recording with infinite overdubs
	- UNDO/REDO function
	- switching looping ON/OFF
	- muting/unmuting
	- resetting
	- adjusting volume

## How it works (will work)

The looper is designed to make recording and playback of loops comfortable and easy in terms of loop length accuracy. For achieving
this the following concepts are implemented:

- _built-in click_ - Tempo (for both phrases) and time signatures (for each phrase separately) are
set before the looper is initialized (before the first track is recorded).
- _scheduled actions_ -  Most of recording and playback related actions are aligned with the click. Instead of action taking
effect immediately after user triggers it via footswitch it is scheduled (flagged) to the next specified event in the click flow
(usually the beginning of new loop cycle). This eliminates the need for precise footswitch presses while ensures loops
to be perfectly aligned with the click. The following are actions with corresponding triggering click events:
	- _start recording of the first track_ - the beginning of bar (according to time signature) after 2 full bars have passed (not counting the bar
	in which the footswitch was pressed) 
	- _stop recording of the first track_ - the first beat of the click after the footswitch was pressed (this means that
	loop lenght is not strictly defined by time signature, user can make also loops which end with incomplete bars)
	- _start recording of other tracks_ - the beginning of loop's new cycle 
	- _stop recording of other tracks_ - recording is automatically stopped at the end of current loop's cycle
	(unless uses stops it manually - this action is not aligned with the click)
	- _switch ON/OFF looping of track_ - the beginning of loop's new cycle
	- _UNDO/REDO of last track's layer_ - the beginning of loop's new cycle

NOTE: The looper may be extended by free-length recording mode without using click in the future to make it's use more
general.

## How it's (will be) controlled

### Momentary switches

There are 4 footswitch boxes (2 for each channel - 1 for recording, 1 for playback) each with 4 switches (1 for each
track of corresponding channel).

- recording switches have following press types mapped to actions:
	- _single press_ - flags/unflags recording of corresponding track if it is not being currently recorded, otherwise
	stops recording
	- _double press_ - N/A
	- _long press_ - flags recording to both phrases simultaneously of corresponding track if it is not being currently
	recorded, otherwise cancels recording
- playback switches have following press types mapped to actions:
	- _single press_ - flags switching looping ON/OFF of corresponding track
	- _double press_ - mutes/unmutes corresponding track
	- _long press_ - UNDO/REDO last recorded layer of corresponding track

There is 1 footswitch box with 3 switches:
- 1 for each channel with following press types mapped to actions:
	- _single press_ - flags/unflags switching looping ON/OFF of corresponding channel
	- _double press_ - mutes/unmutes corresponding channel
	- _long press_ - N/A
- 1 for switching between phases by single pressing

### Pots

All pots are used for adjusting volume. There are:

- 8 pots for tracks
- 2 pots for channels
- 1 pot for click
- 1 pot for main out
- 1 pot for phones 

### Lever switches

There are 2 lever switches, 1 for each channel to switch bypass ON/OFF.

## How it's (will be) put together

The looper logic is implemented in Pure Data patch running on Raspberry Pi 3 while it is being controlled by the sketch
running on Adruino with controlling components connected (footswitches, pots, lever switches, etc.). RPi and
Arduino talk to each other via serial port.

As an audio interface a sound card compatible with RPi which has stereo input and stereo output is required.
[Here](https://puredata.info/docs/raspberry-pi) are some working and not working sound cards listed. I've tried several of
those little cheap USB sound cards with rather bad results. [Behringer
UFO202](http://www.musictri.be/Categories/Behringer/Computer-Audio/Audio-Interfaces/UFO202/p/P0A12) worked quite
well with low latency but so far I had the best results with [Audio Injector](http://www.audioinjector.net/#!/rpi-hat)
that appears to work glitch-free with the latency as low as 5ms. 


   
## Credits

In the initial stage of this project I drew a lot of inspiration from
[GuitarExtended](https://guitarextended.wordpress.com/) blog which provides very comprehensive guide on how to make
guitar effects in Pure Data running on Raspberry Pi. Very well done! Thanks!
