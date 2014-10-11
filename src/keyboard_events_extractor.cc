#include <stdexcept>
#include <algorithm>
#include "keyboard_events_extractor.hh"

static bool is_key_down_event(const struct midi_event& ev)
{
	return (ev.data.size() == 3) and
		((ev.data[0] & 0xF0) == 0x90) and (ev.data[2] != 0x00);
}


static bool is_key_release_event(const struct midi_event& ev)
{
	return (ev.data.size() == 3) and
		(((ev.data[0] & 0xF0) == 0x80) or
		 (((ev.data[0] & 0xF0) == 0x90) and (ev.data[2] == 0x00)));
}

std::vector<struct key_event>
get_key_events(const std::vector<struct midi_event>& midi_events)
{
	// sanity check: precondition: the midi events must be sorted by time
	if (not std::is_sorted(midi_events.begin(), midi_events.end(), [] (const struct midi_event& a, const struct midi_event& b) {
				return a.time < b.time;
			}))
	{
		throw std::invalid_argument("Error: precondition failed. The midi events must be sorted by ordering time.");
	}

	std::vector<struct key_event> res;
	for (const auto& ev : midi_events)
	{
		if (is_key_down_event(ev))
		{
			const struct key_event k = { .time = ev.time,
										 .pitch = ev.data[1],
										 .ev_type = key_event::type::pressed };

			res.push_back(std::move(k));
		}

		if (is_key_release_event(ev))
		{
			const struct key_event k = { .time = ev.time,
										 .pitch = ev.data[1],
										 .ev_type = key_event::type::released };

			res.push_back(std::move(k));			
		}

		// sanity check: an event can't be a key down and a key pressed at the same time
		if (is_key_down_event(ev) and is_key_release_event(ev))
		{
			throw std::invalid_argument("Error, a midi event has been detected as being both a key pressed and key release at the same time");
		}
	}

	// sanity check: the res vector should be sorted by event time
	if (not std::is_sorted(res.begin(), res.end(), [] (const struct key_event& a, const struct key_event& b) {
				return a.time < b.time;
			}))
	{
		throw std::invalid_argument("Error: postcondition failed. The key events must be sorted by ordering time.");
	}
	
	return res;
}
