#include <iostream>
#include <string>

#include "midi_reader.hh"

static void usage(std::ostream& out, const std::string& progname)
{
  out << "Usage: " << progname << " <Option|File>\n"
      << "\n"
      << "Options:\n"
      << "  -h, --help		print this help\n";
}

int main(int argc, char** argv)
{
  const std::string prog_name = argv[0];
  if (argc != 2)
  {
    usage(std::cout, prog_name);
    return 2;
  }

  const std::string arg = argv[1];
  if ((arg == "--help") or (arg == "-h"))
  {
    usage(std::cout, prog_name);
    return 0;
  }

  const auto midi_events = get_midi_events(arg);
  return 0;
}
