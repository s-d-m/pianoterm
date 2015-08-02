PIANOTERM
=========

Pianoterm, as the name suggests, displays a piano in your terminal.
It supports playing midi files, and waiting for events from a midi input port.
Have a look at the following video to get a preview of what it does.

[link-to-video][1]

[1]: ./misc/demo.mp4

The video has no embedded sound in it, but the real program plays it.

License
------

Todo: choose a license

Build dependencies
----------------

`Pianoterm` requires a C++11 compiler to build (clang++-3.5 and g++-4.9 are both fine).
It also depends on the following libraries:

- [`libRtMidi`][rtmidi]
- [`termbox`][termbox]

[rtmidi]: http://www.music.mcgill.ca/~gary/rtmidi/
[termbox]: https://github.com/nsf/termbox

Also note that `pianoterm` does not play music itself. Instead it
relies on a system-wide midi sequencer.  On `GNU/Linux` you might
consider installing `timidity`

On `debian`, one can install them the following way:

	sudo apt-get install timidity librtmidi-dev librtmidi2 g++-4.9

Unfortunately the `termbox` library is not packaged by `debian` so
you need to compile and install it too. To do so:

	git clone --depth 1 'https://github.com/nsf/termbox'
	cd termbox
	./waf configure # requires python
	./waf
	sudo ./waf install


Compiling instructions
-------------------

Once all the dependencies have been installed, you can simply compile `pianoterm` by entering:

	make

This will generate the `pianoterm` binary in `./bin`

How to use
----------

Pianoterm needs a midi sequencer. If you decided to use timidity, you will need to run it first using:

	timity -iA &

Then, you can list the midi "ports" using

	./bin/pianoterm --list

This prints something like:

	5 output ports found:
	  0 -> Midi Through 14:0
	  1 -> TiMidity 128:0
	  2 -> TiMidity 128:1
	  3 -> TiMidity 128:2
	  4 -> TiMidity 128:3

Then you can then play a midi file through one of the former sequencer using:

	./bin/pianoterm --output-port 1 <your_midi_file>

An example midi file is provided in the `misc` folder.

You might also connect a (virtual) keyboard to your computer and use
it in place of the midi file. If such a keyboard is connected it must show up in the listing.
E.g with a [virtual midi keyboard player][vmpk]

[vmpk]: http://sourceforge.net/projects/vmpk/

	./bin/pianoterm --list

This print something like:

	6 output ports found:
	  0 -> Midi Through 14:0
	  1 -> VMPK Input 129:0
	  2 -> TiMidity 130:0
	  3 -> TiMidity 130:1
	  4 -> TiMidity 130:2
	  5 -> TiMidity 130:3

	2 input ports found:
	  0 -> Midi Through 14:0
	  1 -> VMPK Output 128:0

Now run

	./bin/pianoterm --input-port 1 --output-port 2

This will use the virtual keyboark (VMPK) as input, and will use `TiMidity 130:0` as the midi sequencer.

Other files you may want to read
--------------------------------

todo.txt contains a list of things that I still need to do.

Bugs & questions
--------------

Report bugs and questions to da.mota.sam@gmail.com (I trust the anti spam filter)
