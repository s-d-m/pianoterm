#include <algorithm>
#include <stdexcept>
#include "utils.hh"

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
      new_elt.midi_events.push_back(m.data);
      res.push_back(std::move(new_elt));
    }
    else
    {
      elt->midi_events.push_back(m.data);
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
    if ((elt.midi_events.size() == 0) and
	(elt.key_events.size() == 0))
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
    nb_events += elt.midi_events.size() + elt.key_events.size();
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

  return res;
}
