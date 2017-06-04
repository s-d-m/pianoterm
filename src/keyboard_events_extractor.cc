#include <stdexcept>
#include <algorithm>
#include <chrono>
#include "keyboard_events_extractor.hh"
#include "utils.hh"


static
void separate_release_pressed_events(std::vector<struct key_event>& key_events)
{
  // precond the events MUST be sorted by time. this function only works on that case
  if (not std::is_sorted(key_events.begin(), key_events.end(), [] (const key_event& a, const key_event& b) {
	return a.time < b.time;
      }))
  {
    throw std::invalid_argument("Error, events are not sorted by play time");
  }

  // for each pressed event, look if there is another pressed event that happens
  // at the exact same time as its associated release event. If so, shorten the
  // duration of the former pressed event (i.e advance the time the release
  // event occurs).

  // suboptimal implementation in the case the key_events are sorted
  for (auto& k : key_events)
  {
    if (k.data.ev_type == key_data::type::pressed)
    {
      const auto pitch = k.data.pitch;
      const auto earliest_time = k.time;

      // is there a realease happening at the same time?
      auto release_pos = std::find_if(key_events.begin(), key_events.end(), [=] (const key_event& elt) {
	  return (elt.time == earliest_time) and (elt.data.ev_type == key_data::type::released) and (elt.data.pitch == pitch);
	});

      if (release_pos != key_events.end())
      {

	// there do is a release key happening at the same time.
	// Let's find the pressed key responsible for it
	const auto note_start_pos = std::find_if(key_events.rbegin(), key_events.rend(), [=] (const key_event& elt) {
	  return (elt.time < earliest_time) and (elt.data.ev_type == key_data::type::pressed) and (elt.data.pitch == pitch);
	});

	// sanity check: a release event must be preceded by a pressed event.
	if (note_start_pos == key_events.rend())
	{
	  throw std::invalid_argument("error, a there is release event comming from nowhere (failed to find the associated pressed event)");
	}

	// compute the shortening time
	const auto duration = release_pos->time - note_start_pos->time;
	constexpr const std::chrono::nanoseconds max_shortening_time {75000000};

	// shorten the duration by one fourth of its time, in the worst case
	const std::chrono::nanoseconds shortening_time { std::min(static_cast<decltype(duration)>(max_shortening_time.count()),
								  duration / 4) } ;
	release_pos->time -= shortening_time;

      }
    }
  }

  // sanity check: a key release and a key pressed event with the same pitch
  // can't appear at the same time any more
  for (const auto& k : key_events)
  {
    if (k.data.ev_type == key_data::type::released)
    {
      const auto pitch = k.data.pitch;
      const auto time = k.time;

      if (std::any_of(key_events.begin(), key_events.end(), [=] (const struct key_event& a) {
  	    return (a.data.ev_type == key_data::type::pressed) and (a.data.pitch == pitch) and (a.time == time);
  	  }))
      {
	// technically nothing prevents a midi file from having an event appearing twice
	// at the same time. While this should result in nothing special, it messes up
	// with the separate pressed/released event. Indeed, if two release event appear
	// at the exact same time, and the release event should be advanced, only one of
	// them will. Therefore, at the end of the processing, there will still be a
	// pressed/release at the same time.
	//
	// One can argue that this can't happen with proper midi files. Well, truth is
	// that some midi files contain music played by several instruments at the same
	// time. Since pianoterm doesn't keep track of the instrument playing a note,
	// and doesn't filter out any non-piano instrument, it can actually happen with
	// absolutely normal files.  Pianoterm will plays the note for all instruments
	// with a piano. Therefore, it is enough that two instruments stops playing a
	// note at the same time to trigger this problem.
	//
	// One solution would be to remove duplicated events. However this solution
	// would bring other problems, like getting a released (resp. pressed) event
	// without a matching pressed (resp. released).
	//
	// Another one would be to advance all identic released events. This solution
	// could also bring other problems/
	//
	// Due to the goals of pianoterm, if the problem of duplicate events appears,
	// the file will simply be rejected instead of "automagically fixed".

  	throw std::invalid_argument("Error: a key is said to be pressed and released at the same time");
      }
    }
  }


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
      res.emplace_back(ev.time /* time */,
		       ev.data[1] /* pitch */,
		       key_data::type::pressed /* event type */);
    }

    if (is_key_release_event(ev))
    {
      res.emplace_back(ev.time /* time */,
		       ev.data[1] /* pitch */,
		       key_data::type::released /* event type */);
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

  separate_release_pressed_events(res);
  // res is not sorted anymore (because some release events got advanced)

  return res;
}
