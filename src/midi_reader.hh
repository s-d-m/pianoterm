#ifndef MIDI_READER_HH_
#define MIDI_READER_HH_

#include <vector>
#include <string>
#include <limits>
#include <chrono>

struct midi_event
{
    std::chrono::nanoseconds time;
    std::vector<uint8_t> data;

    midi_event()
      : time (std::numeric_limits<decltype(time)>::max())
      , data ()
    {
    }
};

std::vector<struct midi_event>
get_midi_events(const std::string& filename);

#endif /* MIDI_READER_HH_ */
