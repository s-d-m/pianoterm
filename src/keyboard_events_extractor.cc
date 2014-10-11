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


static
void separate_release_pressed_events(std::vector<struct key_event>& key_events)
{
  // for each pressed event, look if there is another pressed event that happens
  // at the exact same as the associated release event if so, shorten the
  // duration of the former pressed event (i.e advance the time the release
  // event occurs).

  // suboptimal implementation in the case the key_events are sorted
  for (auto& k : key_events)
  {
    if (k.ev_type == key_event::type::pressed)
    {
      // find the associated key_release
      const auto pitch = k.pitch;
      const auto earliest_time = k.time;

      auto release_ev = std::find_if(key_events.begin(), key_events.end(), [=] (const struct key_event& elt) {
	  return (elt.time > earliest_time) and (elt.ev_type == key_event::type::released) and (elt.pitch == pitch);
	});

      // sanity check: a pressed key must be followed by a release key with the same pitch
      if (release_ev == key_events.end())
      {
	throw std::invalid_argument("Error, a pressed key was never released");
      }

      // let's see if there is a key pressed with that pitch that is played
      // at the exact same time as the key release event. This can happen
      // when e.g. two quarter notes are following each others. We need to
      // introduce a small time gap in between as otherwise the user can't
      // _visually_ differentiate between the two quarter notes being played
      // successfully or only one half note being played.
      const auto release_time = release_ev->time;
      if (std::any_of(key_events.begin(), key_events.end(), [=] (const struct key_event& elt) {
	    return (elt.time == release_time) and (elt.pitch == pitch) and (elt.ev_type == key_event::type::pressed);
	  }))
      {
	// need to advance the key release event
	const auto duration = release_ev->time - earliest_time;
	const auto max_shortening_time = decltype(duration){75000000}; // nanoseconds

	// shorten the duration by one fourth of its time, in the worst case
	const auto shortening_time = std::min(max_shortening_time, duration / 4);
	release_ev->time -= shortening_time;
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

  separate_release_pressed_events(res);
  // res is no more sorted not (because some release events got advanced)

  return res;
}
