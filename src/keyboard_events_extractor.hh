#ifndef KEYBOARD_EVENTS_EXTRACTOR_HH_
#define KEYBOARD_EVENTS_EXTRACTOR_HH_

#include <vector>
#include "midi_reader.hh"

struct key_event
{
	enum type : bool
	{
		pressed,
		released,
	};
	
	uint64_t time; // the time the event occurs during the sond
	uint8_t pitch; // the key that is pressed or released
	type    ev_type; // was the key pressed or released?
};

// extracts the key_pressed / key_released from the midi events
std::vector<struct key_event>
get_key_events(const std::vector<struct midi_event>& midi_events);


#endif /* KEYBOARD_EVENTS_EXTRACTOR_HH_ */
