#ifndef UTILS_HH_
#define UTILS_HH_

#include <vector>
#include <limits>
#include "midi_reader.hh"
#include "keyboard_events_extractor.hh"

using midi_message = std::vector<uint8_t>;

struct music_event
{
    uint64_t time; // occuring time
    std::vector<midi_message> midi_events;
    std::vector<struct key_data> key_events;

    music_event()
      : time (std::numeric_limits<decltype(time)>::max())
      , midi_events ()
      , key_events ()
    {
    }
};

// a song is just a succession of music_event to be played
std::vector<struct music_event>
group_events_by_time(const std::vector<struct midi_event>& midi_events,
		     const std::vector<struct key_event>& key_events);

#endif /* UTILS_HH_ */
