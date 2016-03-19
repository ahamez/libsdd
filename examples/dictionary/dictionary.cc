#include <iostream>
#include <fstream>
#include <numeric> // iota
#include <vector>

#include "sdd/sdd.hh"
#include "sdd/tools/dot/sdd.hh"

/*------------------------------------------------------------------------------------------------*/

struct conf
  : public sdd::flat_set_default_configuration
{
  using Identifier = unsigned int;
  using Values     = sdd::values::flat_set<char>;
};
using SDD         = sdd::SDD<conf>;
using values_type = conf::Values;

/*------------------------------------------------------------------------------------------------*/

int
main(int argc, char** argv)
{
  // Intialize the library.
  auto manager = sdd::init<conf>();

  // We need a dictionary.
  if (argc < 2)
  {
    std::cerr << "Missing dictionary file" << std::endl;
    return 1;
  }

  // Open dictionary.
  std::ifstream dict_file(argv[1]);
  if (not dict_file.is_open())
  {
    std::cerr << "Can't read " << argv[1] << std::endl;
    return 2;
  }

  // Optional DOT output file.
  std::string dot_file_path;
  if (argc == 3)
  {
    dot_file_path.assign(argv[2]);
  }

  // Will hold a line of the file
  std::string line;
  line.reserve(256);

  // Find the max size of a word.
  std::size_t max_size = 0;
  std::size_t nb_lines = 0;
  while (std::getline(dict_file, line))
  {
    max_size = max_size > line.size() ? max_size : line.size();
    ++nb_lines;
  }

  // Rewind file.
  dict_file.clear();
  dict_file.seekg(0);

  // Construct the SDD order: we need one level per letter.
  std::vector<unsigned int> v(max_size);
  std::iota(v.begin(), v.end(), 0);
  sdd::order_builder<conf> ob;
  const sdd::order<conf> order(sdd::order_builder<conf>(v.begin(), v.end()));

  // Construct the SDD dictionary.
  SDD dict = sdd::zero<conf>();
  std::size_t loop = 0;
  while (std::getline(dict_file, line))
  {
    dict += SDD(order, [&](unsigned int pos)
                          {
                            return pos < line.size() ? values_type {line[pos]} : values_type {'#'};
                          });
    std::cout << "\r" << loop++ << "/" << nb_lines;
  }
  std::cout << std::endl;
  std::cout << dict.size() << " encoded words" << std::endl;
  
  if (not dot_file_path.empty())
  {
    std::ofstream dot_file(dot_file_path);
    if (not dot_file.is_open())
    {
      std::cerr << "Can't write " << dot_file_path << std::endl;
      return 3;
    }
    dot_file << sdd::tools::dot(dict, order);
  }

  return 0;
}
