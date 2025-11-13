#include "model/model.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char **argv) {
  assert(argc == 2);
  model::raw_model_t raw_model = model::load_model_from_path(argv[1]);
  std::cout << raw_model.meshes.size() << '\n';

  raw_model = model::merge_meshes(raw_model);

  auto triangles = model::create_triangles_from_mesh(raw_model.meshes[0]);

  for (auto triangle : triangles) {
    if (triangle.area() == 0) {
      std::cout << triangle << '\n';
      throw std::runtime_error("degen triangle");
    }
  }

  return 0;
}
