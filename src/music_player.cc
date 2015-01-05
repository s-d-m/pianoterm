#include <termbox.h>
#include <RtMidi.h>
#include <stdexcept>
extern "C" {
#include <sys/time.h> // for gettimofday
}

#include <signal.h> // for sig_atomic_t type

#include "music_player.hh"
#include "keyboard_events_extractor.hh"

// Global variables to "share" state between the signal handler and
// the main event loop.  Only these two pieces should be allowed to
// use these global variables.  To avoid any other piece piece of code
// from using it, the declaration is not written on a header file on
// purpose.
extern volatile sig_atomic_t interrupt_required;
extern volatile sig_atomic_t continue_required;
extern volatile sig_atomic_t exit_required;

static
void draw_piano_key(int x, int y, int width, int height,  uint16_t color)
{
  for (int i = x; i < x + width; ++i)
  {
    for (int j = y; j < y + height; ++j)
    {
      tb_change_cell(i, j, 0x2588, color, TB_DEFAULT);
    }
  }
}

static void draw_separating_line(int x, int y, int height, uint16_t bg_color)
{
  for (int j = y; j < y + height; ++j)
  {
    tb_change_cell(x, j, 0x2502, TB_BLACK, bg_color);
  }
}

struct octave_color
{
    octave_color()
      : do_color (TB_WHITE)
      , re_color (TB_WHITE)
      , mi_color (TB_WHITE)
      , fa_color (TB_WHITE)
      , sol_color (TB_WHITE)
      , la_color (TB_WHITE)
      , si_color (TB_WHITE)

      , do_diese_color (TB_BLACK)
      , re_diese_color (TB_BLACK)
      , fa_diese_color (TB_BLACK)
      , sol_diese_color (TB_BLACK)
      , la_diese_color (TB_BLACK)
    {
    }

    uint8_t do_color;
    uint8_t re_color;
    uint8_t mi_color;
    uint8_t fa_color;
    uint8_t sol_color;
    uint8_t la_color;
    uint8_t si_color;

    uint8_t do_diese_color;
    uint8_t re_diese_color;
    uint8_t fa_diese_color;
    uint8_t sol_diese_color;
    uint8_t la_diese_color;
};

struct keys_color
{
    keys_color()
      : la_0_color (TB_WHITE)
      , si_0_color (TB_WHITE)
      , la_diese_0_color (TB_BLACK)
      ,	octaves ()
      , do_8_color (TB_WHITE)
    {
    }

    uint8_t la_0_color;
    uint8_t si_0_color;
    uint8_t la_diese_0_color;
    struct octave_color octaves[7];
    uint8_t do_8_color;
};

static
void draw_octave(int x, int y, const struct octave_color& notes_color)
{
  draw_piano_key(x,     y, 3, 8, notes_color.do_color);  // do
  draw_piano_key(x + 3, y, 4, 8, notes_color.re_color);  // re
  draw_piano_key(x + 7, y, 3, 8, notes_color.mi_color);  // mi

  draw_piano_key(x + 10, y, 4, 8, notes_color.fa_color); // fa
  draw_piano_key(x + 14, y, 4, 8, notes_color.sol_color); // sol
  draw_piano_key(x + 18, y, 3, 8, notes_color.la_color); // la
  draw_piano_key(x + 21, y, 4, 8, notes_color.si_color); // si

  draw_piano_key(x + 2, y, 2, 5, notes_color.do_diese_color);  // do#
  draw_piano_key(x + 6, y, 2, 5, notes_color.re_diese_color);  // re#

  draw_separating_line(x + 3, y + 5, 3, notes_color.do_color); // between do and re
  draw_separating_line(x + 6, y + 5, 3, notes_color.re_color); // between re and mi

  draw_piano_key(x + 13, y, 2, 5, notes_color.fa_diese_color); // fa#
  draw_piano_key(x + 17, y, 2, 5, notes_color.sol_diese_color); // sol#
  draw_piano_key(x + 21, y, 2, 5, notes_color.la_diese_color); // la#

  draw_separating_line(x + 14, y + 5, 3, notes_color.fa_color); // between fa and sol
  draw_separating_line(x + 18, y + 5, 3, notes_color.sol_color); // between sol and la
  draw_separating_line(x + 21, y + 5, 3, notes_color.la_color); // between la and si

  draw_separating_line(x + 10, y, 8, notes_color.mi_color); // between mi and fa

}

static void draw_keyboard(const struct keys_color& keyboard, int pos_x, int pos_y)
{
  draw_piano_key(pos_x + 1, pos_y, 3, 8, keyboard.la_0_color); // la 0
  draw_piano_key(pos_x + 4, pos_y, 4, 8, keyboard.si_0_color); // si 0
  draw_piano_key(pos_x + 4, pos_y, 2, 5, keyboard.la_diese_0_color); // la# 0
  draw_separating_line(pos_x + 4, pos_y + 5, 3, keyboard.la_0_color); // between la0 and si0

  for (int i = 0; i < 7; ++i)
  {
    draw_octave(pos_x + 8 + (25 * i), pos_y, (keyboard.octaves[i]));
  }

  draw_piano_key(pos_x + 8 + (25 * 7), pos_y, 4, 8, keyboard.do_8_color); // do 8

  for (int i = 0; i < 7; ++i)
  {
    draw_separating_line(pos_x + 8 + (25 * (i + 1)), pos_y, 8, keyboard.octaves[i].si_color); // between octaves
  }
  draw_separating_line(pos_x + 8 + (25 * 0), pos_y, 8, keyboard.si_0_color);
}


#define octave_color(X)						\
  case note_kind::do_##X:					\
  keyboard.octaves[(X - 1)].do_color = normal_key_color;	\
  break;							\
  case note_kind::do_diese##X:					\
  keyboard.octaves[(X - 1)].do_diese_color = diese_key_color;	\
  break;							\
  case note_kind::re_##X:					\
  keyboard.octaves[(X - 1)].re_color = normal_key_color;	\
  break;							\
  case note_kind::re_diese_##X:					\
  keyboard.octaves[(X - 1)].re_diese_color = diese_key_color;	\
  break;							\
  case note_kind::mi_##X:					\
  keyboard.octaves[(X - 1)].mi_color = normal_key_color;	\
  break;							\
  case note_kind::fa_##X:					\
  keyboard.octaves[(X - 1)].fa_color = normal_key_color;	\
  break;							\
  case note_kind::fa_diese_##X:					\
  keyboard.octaves[(X - 1)].fa_diese_color = diese_key_color;	\
  break;							\
  case note_kind::sol_##X:					\
  keyboard.octaves[(X - 1)].sol_color = normal_key_color;	\
  break;							\
  case note_kind::sol_diese_##X:				\
  keyboard.octaves[(X - 1)].sol_diese_color = diese_key_color;	\
  break;							\
  case note_kind::la_##X:					\
  keyboard.octaves[(X - 1)].la_color = normal_key_color;	\
  break;							\
  case note_kind::la_diese_##X:					\
  keyboard.octaves[(X - 1)].la_diese_color = diese_key_color;	\
  break;							\
  case note_kind::si_##X:					\
  keyboard.octaves[(X - 1)].si_color = normal_key_color;	\
  break								\


#if defined(__clang__)
  // clang will complain in a switch that the default case is useless
  // because all possible values in the enum are already taken into
  // account.  however, since the value can come from an unrestricted
  // uint8_t, the default is actually a necessary safe-guard.
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wcovered-switch-default"
#endif

static void set_color(struct keys_color& keyboard, enum note_kind note, uint8_t normal_key_color, uint8_t diese_key_color)
{
  switch (note)
  {
    case note_kind::la_0:
      keyboard.la_0_color = normal_key_color;
      break;

    case note_kind::la_diese_0:
      keyboard.la_diese_0_color = diese_key_color;
      break;

    case note_kind::si_0:
      keyboard.si_0_color = normal_key_color;
      break;

      octave_color(1);
      octave_color(2);
      octave_color(3);
      octave_color(4);
      octave_color(5);
      octave_color(6);
      octave_color(7);

    case note_kind::do_8:
      keyboard.do_8_color = normal_key_color;
      break;

    default:
      std::cerr << "Warning key " << static_cast<long unsigned int>(note)
		<< " is not representable in a 88 key piano" << std::endl;
      break;
  }
}
#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

#undef octave_color

static void set_color(struct keys_color& keyboard, enum note_kind note)
{
  set_color(keyboard, note, TB_BLUE, TB_CYAN);
}

static void reset_color(struct keys_color& keyboard, enum note_kind note)
{
  set_color(keyboard, note, TB_WHITE, TB_BLACK);
}



template <typename T>
static
void init_sound(T& player, unsigned int midi_port)
{

  player.openPort(midi_port);

  if (!player.isPortOpen())
  {
    throw std::runtime_error("Error while initialising the sound system: couldn't open output sound port");
  }
}

static
void init_termbox()
{
  /* initialize the window */
  switch (tb_init())
  {
    case TB_EUNSUPPORTED_TERMINAL:
      throw std::runtime_error("Failed to initialize the `termbox` module (the plain console UI) because the terminal is not supported");

    case TB_EFAILED_TO_OPEN_TTY:
      throw std::runtime_error("Failed to initialize the `termbox` module (the plain console UI) (couldn't open TTY)");

    case TB_EPIPE_TRAP_ERROR:
      throw std::runtime_error("Failed to initialize the `termbox` module (the plain console UI) (pipe trap error)");

    default:
      /* no errors, keep going. */
      break;
  }
}

static void update_keyboard(struct keys_color& keyboard, const std::vector<key_data>& key_events)
{
      /* update the keyboard */
    for (const auto& k_ev : key_events)
    {
      switch (k_ev.ev_type)
      {
	case key_data::type::pressed:
    	  set_color(keyboard, static_cast<enum note_kind>(k_ev.pitch));
	  break;

	case key_data::type::released:
    	  reset_color(keyboard, static_cast<enum note_kind>(k_ev.pitch));
	  break;

#if !defined(__clang__)
// clang complains that all values are handled in the switch and issue
// a warning for the default case
// gcc complains about a missing default
	default:
	  break;
#endif
      }
    }
}


// following function was copy/pasted from the termbox computer keyboard example
static void print_tb(const char *str, int x, int y, uint16_t fg, uint16_t bg)
{
  while (*str) {
    uint32_t uni;
    str += tb_utf8_char_to_unicode(&uni, str);
    tb_change_cell(x, y, uni, fg, bg);
    x++;
  }
}

static void update_screen(const struct keys_color& keyboard, const int ref_x, const int ref_y)
{

    /* draw keyboard */
    tb_clear();
    draw_keyboard(keyboard, ref_x, ref_y);

    print_tb("press <CTRL + q> to quit", ref_x, ref_y + 10, TB_MAGENTA, TB_DEFAULT);
    print_tb("press <space> to pause/unpause", ref_x, ref_y + 11, TB_MAGENTA, TB_DEFAULT);

    tb_present();

}


static void play_music(RtMidiOut& sound_player, const std::vector<midi_message>& midi_messages)
{
  // play the music
  for (const auto& message : midi_messages)
  {
    auto tmp = message; // can't use message directly since message is const and
			// sendMessage doesn't take a const vector
    sound_player.sendMessage(&tmp);

    // could use the following to cast the const away: but since there is no
    // guarantee that the libRtMidi doesn't modify the data ... (I know the I
    // can read the code)
    //
    //sound_player.sendMessage(const_cast<midi_message*>(&message));
  }
}

static
void init_ref_pos(int& ref_x, int& ref_y, int width, int height)
{
  /* to center the keyboard on the window */
  const auto keyboard_height = 8;
  const auto keyboard_width = 188;
  ref_x = (width - keyboard_width) / 2;
  ref_y = (height - keyboard_height ) / 2;
}

static
void init_ref_pos(int& ref_x, int& ref_y)
{
  init_ref_pos(ref_x, ref_y, tb_width(), tb_height());
}

void play(const std::vector<struct music_event>& music, unsigned int midi_output_port)
{
  RtMidiOut sound_player (RtMidi::LINUX_ALSA);

  init_sound(sound_player, midi_output_port);
  SCOPE_EXIT_BY_REF(sound_player.closePort());

  init_termbox();
  SCOPE_EXIT(tb_shutdown());

  struct keys_color keyboard;

  int ref_x;
  int ref_y;
  init_ref_pos(ref_x, ref_y);


  /* start playing the events */
  const auto nb_events = music.size();
  for (unsigned i = 0; i < nb_events; ++i)
  {
    const auto& current_event = music[i];

    update_keyboard(keyboard, current_event.key_events);
    update_screen(keyboard, ref_x, ref_y);
    play_music(sound_player, current_event.midi_messages);

    if (i != nb_events - 1)
    {
      // sleep until next music event or a key (== space or ctrl+q) is pressed
      const decltype(music[0].time) time_to_wait = (music[i + 1].time - current_event.time) / 1000; // sleep time is in micro second
      decltype(music[0].time) waited_time = 0;

      struct timeval timeval;
      gettimeofday(&timeval, nullptr);
      const auto started_time = static_cast<uint64_t>(timeval.tv_sec * 1000000 + timeval.tv_usec);

      do
      {
	struct tb_event tmp;
	auto ret_val = tb_peek_event(&tmp, static_cast<int>((time_to_wait - waited_time) / 1000)); // tb_peek_event has a timeout set in milliseconds
	switch (ret_val)
	{
	  case TB_EVENT_KEY:
	    switch (tmp.key)
	    {
	      case TB_KEY_CTRL_Q:
		return; // ctrl + q means quit

	      case TB_KEY_SPACE:
		// pause, wait for a second space being pressed (to unpause) or a ctrl+q to quit
	      {
		for (;;)
		{
		  if (tb_poll_event(&tmp) == TB_EVENT_KEY)
		  {
		    if (tmp.key == TB_KEY_SPACE)
		    {
		      break;
		    }

		    if (tmp.key == TB_KEY_CTRL_Q)
		    {
		      return;
		    }
		  }

		}
		break;
	      }
	      default:
		break;
	    }
	    break;

	  case TB_EVENT_RESIZE:
	    init_ref_pos(ref_x, ref_y, tmp.w, tmp.h);
	    break;

	  default:
	    break;
	}

	gettimeofday(&timeval, nullptr);
	const auto time_now = static_cast<uint64_t>(timeval.tv_sec * 1000000 + timeval.tv_usec);
	waited_time = time_now - started_time;
      } while (waited_time < time_to_wait);
    }
  }
}



struct callback_data_t
{
    struct keys_color& keyboard;
    RtMidiOut& sound_player;
    int& ref_x;
    int& ref_y;
};

static
void callback_fn(double timestamp __attribute__((unused)), std::vector<unsigned char> *message, void* param) {
  if (message == nullptr)
  {
    throw std::invalid_argument("Error, invalid input message");
  }

  if (param == nullptr)
  {
    throw std::invalid_argument("Error, argument for input listener");
  }

  auto priv_data = static_cast<struct callback_data_t*>(param);

  std::vector<midi_message> tmp;
  tmp.push_back(*message);

  play_music(priv_data->sound_player, tmp);

  const auto key_events = midi_to_key_events(*message);
  update_keyboard(priv_data->keyboard, key_events);
  update_screen(priv_data->keyboard, priv_data->ref_x, priv_data->ref_y);
}

void play(unsigned int midi_input_port, unsigned int midi_output_port)
{
  RtMidiIn sound_listener (RtMidi::LINUX_ALSA);
  init_sound(sound_listener, midi_input_port);
  SCOPE_EXIT_BY_REF(sound_listener.closePort());

  RtMidiOut sound_player (RtMidi::LINUX_ALSA);
  init_sound(sound_player, midi_output_port);
  SCOPE_EXIT_BY_REF(sound_player.closePort());

  init_termbox();
  SCOPE_EXIT(tb_shutdown());


  struct keys_color keyboard;

  int ref_x;
  int ref_y;
  init_ref_pos(ref_x, ref_y);
  update_screen(keyboard, ref_x, ref_y);


  struct callback_data_t callback_data =  { .keyboard = keyboard,
					    .sound_player = sound_player,
					    .ref_x = ref_y,
					    .ref_y = ref_y };


  sound_listener.setCallback(callback_fn, &callback_data);

  // This mode is playing from a midi keyboard as input, not a midi
  // file.  In this mode there is play/pause. It wouldn't make
  // sense. Therefore the event loop will only look at a signal
  // requiring to shutdown the program, and will happily ignore any
  // SIGINT/SIGCONT it might receive.

  for (;;)
  {
    if (exit_required)
    {
      return;
    }

    struct tb_event ev;

    // use peek_event with a _small_ timeout instead of waiting forever
    // as we may need to quit if we got a signal requesting so.
    const auto ret_val = tb_peek_event(&ev, 100 /* timeout in ms */);

    switch (ret_val)
    {
      case TB_EVENT_RESIZE:
	init_ref_pos(ref_x, ref_y, ev.w, ev.h);
	update_screen(keyboard, ref_x, ref_y);
	break;
      case TB_EVENT_KEY:
	if (ev.key == TB_KEY_CTRL_Q)
	{
	  return; // ctrl + q means quit
	}
	break;
      default:
	break;
    }
  }
}
