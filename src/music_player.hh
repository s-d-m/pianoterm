#ifndef MUSIC_PLAYER_HH_
#define MUSIC_PLAYER_HH_

#include "utils.hh"

void play(const std::vector<struct music_event>& music, unsigned int midi_output_port);

#endif /* MUSIC_PLAYER_HH_ */
