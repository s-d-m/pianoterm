#include <algorithm>
#include <stdexcept>
#include "utils.hh"

bool is_key_down_event(const std::vector<uint8_t>& data)
{
  return (data.size() == 3) and
         ((data[0] & 0xF0) == 0x90) and (data[2] != 0x00);
}

bool is_key_down_event(const struct midi_event& ev)
{
  return is_key_down_event(ev.data);
}

bool is_key_release_event(const std::vector<uint8_t>& data)
{
  return (data.size() == 3) and
    (((data[0] & 0xF0) == 0x80) or
     (((data[0] & 0xF0) == 0x90) and (data[2] == 0x00)));
}

bool is_key_release_event(const struct midi_event& ev)
{
  return is_key_release_event(ev.data);
}

// in case there is a release pitch and a play pitch at the same time
// in the midi part, make sure the release happens *before* the play.
// the release must be from a previous play as otherwise it would mean
// that the a key is pressed and immediately released, so not played
// at all (which is wrong)
static
void fix_midi_order(std::vector<struct music_event>& music)
{
  for (auto& music_event : music)
  {
    auto& messages = music_event.midi_messages;

    const auto messages_begin = messages.end();
    const auto messages_end = messages.end();

    for (auto it = messages_begin; it != messages_end; ++it)
    {
      auto& midi_ev = *it;
      if (is_key_release_event(midi_ev))
      {
	const auto pitch = midi_ev[1];
	auto down_pos = std::find_if(std::next(it), messages_end, [=] (const midi_message& elt) {
	    return is_key_down_event(elt) and (elt[1] == pitch);
	  });

	// no need to check if down pos == end, since distance from
	// begin to end is bigger than distance from begin to it
	if (std::distance(messages_begin, down_pos) < std::distance(messages_begin, it))
	{
	  std::swap(*it, *down_pos);
	}

      }
    }
  }
}

// a song is just a succession of music_event to be played
std::vector<struct music_event>
group_events_by_time(const std::vector<struct midi_event>& midi_events,
		     const std::vector<struct key_event>& key_events)
{
  std::vector<struct music_event> res;

  // totally suboptimal implementation
  for (const auto& m : midi_events)
  {
    const auto ev_time = m.time;
    auto elt = std::find_if(res.begin(), res.end(), [=] (const struct music_event& a) {
	return a.time == ev_time;
      });

    if (elt == res.end())
    {
      struct music_event new_elt;
      new_elt.time = ev_time;
      new_elt.midi_messages.push_back(m.data);
      res.push_back(std::move(new_elt));
    }
    else
    {
      elt->midi_messages.push_back(m.data);
    }
  }


  for (const auto& k : key_events)
  {
    const auto ev_time = k.time;
    auto elt = std::find_if(res.begin(), res.end(), [=] (const struct music_event& a) {
	return a.time == ev_time;
      });

    if (elt == res.end())
    {
      struct music_event new_elt;
      new_elt.time = ev_time;
      new_elt.key_events.push_back(k.data);
      res.push_back(std::move(new_elt));
    }
    else
    {
      elt->key_events.push_back(k.data);
    }
  }

  std::sort(res.begin(), res.end(), [] (const struct music_event& a, const struct music_event& b) {
      return a.time < b.time;
    });

  // sanity check: all elts in res must hold at least one event
  for (const auto& elt : res)
  {
    if (elt.midi_messages.empty() and elt.key_events.empty())
    {
      throw std::invalid_argument("Error: a music event does not contain any midi or key event");
    }
  }

  // sanity check: worst case res has as many elts as midi_events + key_events
  // (each event occuring at a different time)
  const auto nb_input_events = midi_events.size() + key_events.size();
  if (res.size() > nb_input_events)
  {
    throw std::invalid_argument("Error while grouping events by time, some events just got automagically created");
  }

  // sanity check: count the total number of midi and key events in res. It must
  // match the number of parameters given in the parameters
  uint64_t nb_events = 0;
  for (const auto& elt : res)
  {
    nb_events += elt.midi_messages.size() + elt.key_events.size();
  }

  if (nb_events > nb_input_events)
  {
    throw std::invalid_argument("Error while grouping events by time, some events magically appeared");
  }

  if (nb_events < nb_input_events)
  {
    throw std::invalid_argument("Error while grouping events by time, some events just disappeared");
  }

  // sanity check: for every two different elements in res, they must start at different time
  // since res is sorted by now, only need to check
  for (auto i = decltype(res.size()){1}; i < res.size(); ++i)
  {
    if (res[i].time == res[i - 1].time)
    {
      throw std::invalid_argument("Error two different group of events appears at the same time");
    }
  }

  // sanity check: there must be as many release events as pressed events
  uint64_t nb_released = 0;
  uint64_t nb_pressed = 0;
  for (const auto& elt : res)
  {
    for (const auto& k : elt.key_events)
    {
      if (k.ev_type == key_data::type::released)
      {
	nb_released++;
      }
      if (k.ev_type == key_data::type::pressed)
      {
	nb_pressed++;
      }
    }
  }

  if (nb_pressed != nb_released)
  {
    throw std::invalid_argument("Error: mismatch between key pressed and key release");
  }

  // sanity check: a key release and a key pressed event with the same pitch
  // can't appear at the same time
  for (const auto& elt : res)
  {
    for (const auto& k : elt.key_events)
    {
      if (k.ev_type == key_data::type::released)
      {
  	const auto pitch = k.pitch;
  	if (std::any_of(elt.key_events.begin(), elt.key_events.end(), [=] (const struct key_data& a) {
  	      return (a.ev_type == key_data::type::pressed) and (a.pitch == pitch);
  	    }))
  	{
  	  throw std::invalid_argument("Error: a key press happens at the same time as a key release");
  	}
      }
    }
  }

  fix_midi_order(res);

  return res;
}
