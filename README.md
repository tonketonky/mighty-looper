Mighty Looper project is an attempt to build low-cost 2-channel multi-track looping machine that allows to operate
within 2 individual loop phrases (verse/chorus). 

The looper logic is implemented in Pure Data patch running on Raspberry Pi 3 while it is being controled by the sketch
running on Adruino with the controlling components connected (footswitches, pots, toggle switches, etc.). RPI and
Arduino talk to each other via serial port.

As an audio interface a sound card compatible with RPI which has stereo input and stereo output channels is required.
Here are some working and not working sound cards listed: https://puredata.info/docs/raspberry-pi. I've tried several of
those little cheap USB sound cards with rather bad results and so far I ended up with Behringer UFO202 that works quite
well with low latency. Anyways the sound card is pretty easy part of the setup to replace. 

Currently the project is still in the stage of development. Almost none of the hardware part is completed, a lot of
features in the looper logic and the most of looper controls are not implemented yet. Also the present code needs a
refactoring since I've been basically learning both Arduino and Pure Data from scratch by writting this project.

Here is the list of features and functionalities of Migthy Looper that I aim to achieve:

- 2 separate channels each with: 
	- volume control 
	- toggling playback while recording ON an OFF
- 4 separate tracks for each channel
- individual control of each track involves:  
	- recording
	- infinite overdubs
	- UNDO/REDO function
	- toggling playback ON and OFF
	- resetting
	- adjusting volume
- 2 separate phrases (verse/chorus) possible to switch between
- individual reset and start over for each phrase
- switching 2 recording modes:
	- Free length mode:	the length of the verse is determined 
											by the first track of the verse recorded
	- Fixed length mode: 	the length of the verse by predetermined
												by setting tempo and time signature, this
												mode requires using "MONO" output mode with
												click (see below)                              
- switching 2 output modes:
	- STEREO mode: 	each channel in a separate output
	- MONO mode: 	both channels mixed together in both outputs, 
								while one of the outputs (intended for headphones) 
								is capable of turning click ON and OFF in case 
								of "Fixed length" recording mode selected
   
NOTE: Each of the signals for controlling tracks (record, playback ON/OFF, 
			UNDO/REDO, except for starting and stopping recording of the first 
			track of a given verse in "Free length" recording mode) takes effect at the 
			beginning of next cycle of the loop. 
