#ifndef MUSIC_PLAYER_HH_
#define MUSIC_PLAYER_HH_

#include "utils.hh"


void play(const std::vector<struct music_event>& music, unsigned int midi_output_port);

// listen to a midi input, plays it to output
void play(unsigned int midi_input_port, unsigned int midi_output_port);

#endif /* MUSIC_PLAYER_HH_ */
