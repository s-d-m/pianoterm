#include <iostream>
#include <string>

#include "midi_reader.hh"
#include "keyboard_events_extractor.hh"
#include "utils.hh"
#include "music_player.hh"

struct options
{
    bool has_error;
    bool print_help;
    bool list_ports;
    unsigned int output_port;
    std::string filename;

    options()
      : has_error (false)
      , print_help (false)
      , list_ports (false)
      , output_port (get_default_output_port())
      , filename ("")
    {
    }
};

static
struct options get_opts(int argc, char** argv)
{
  struct options res;

  for (int i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];
    if ((arg == "-h") or (arg == "--help"))
    {
      res.print_help = true;
      continue;
    }

    if ((arg == "-l") or (arg == "--list"))
    {
      res.list_ports = true;
      continue;
    }

    if ((arg == "-p") or (arg == "-port"))
    {
      if (i == argc - 1)
      {
	res.has_error = true;
	return res;
      }
      else
      {
	++i;
	res.output_port = get_output_port(argv[i]);
      }
      continue;
    }

    if (res.filename != "")
    {
      res.has_error = true;
      return res;
    }
    else
    {
      res.filename = argv[i];
    }
  }

  return res;
}

static void usage(std::ostream& out, const std::string& progname)
{
  out << "Usage: " << progname << " [Options] [File]\n"
      << "\n"
      << "Options:\n"
      << "  -h, --help		print this help\n"
      << "  -l, --list		list the midi output ports available for use\n"
      << "  -p, --port <NUM>	the output midi port to use\n";
}



int main(int argc, char** argv)
{
  const auto opts = get_opts(argc, argv);

  const std::string prog_name = argv[0];

  if (opts.has_error)
  {
    usage(std::cerr, prog_name);
    return 2;
  }

  if (opts.print_help)
  {
    usage(std::cout, prog_name);
    return 0;
  }

  if (opts.list_ports)
  {
    list_output_midi_ports(std::cout);
    return 0;
  }

  if (opts.filename == "")
  {
    usage(std::cerr, prog_name);
    return 2;
  }

  try
  {
    const auto midi_events = get_midi_events(opts.filename);
    const auto keyboard_events = get_key_events(midi_events);
    const auto song = group_events_by_time(midi_events, keyboard_events);

    play(song, opts.output_port);
  }
  catch (std::exception& e)
  {
    std::cerr << e.what();
    return 2;
  }
}
