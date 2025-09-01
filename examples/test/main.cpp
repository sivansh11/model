#include "model/model.hpp"

#include <iostream>

int main(int argc, char **argv) {
  assert(argc == 2);
  model::raw_model_t raw_model = model::load_model_from_path(argv[1]);
  std::cout << raw_model.meshes.size() << '\n';
  return 0;
}
