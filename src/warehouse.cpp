#include "warehouse.hpp"


std::vector<std::vector<double>> getAisleEnds(const int loc_idx,
  const std::vector<std::vector<double>>& loc_mat,
  const std::vector<double>& wh_params) {
  double w_cross_aisle = wh_params[0];
  double loc_wy = wh_params[1];
  int num_locs = wh_params[2];
  if (loc_idx == 0) {
    std::cout << "myWarning: depot selected as input for getting aisle ends" << std::endl;
    return {{0, 0}, {0, 0}};
  }
  double x = loc_mat[loc_idx][1];
  int block_idx = loc_mat[loc_idx][3];
  // y coord is always a multiple of block size
  double Yblock = w_cross_aisle + loc_wy * num_locs;
  double ya = block_idx * Yblock;  // start of aisle (below)
  double yb = (block_idx + 1) * Yblock;  // end of aisle (above)

  return {{x, ya}, {x, yb}};
}

