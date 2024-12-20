/**
* @file
* @version 0.0
*
* @section DESCRIPTION
* Warehouse-related functions. Assumes a multi-block rectilinear layout.
*/
#ifndef WAREHOUSE_HPP
#define WAREHOUSE_HPP

#include <vector>
#include <variant>
#include <fstream>
#include <iostream>
#include <string>

/**
* @brief Get coordinates of the aisle ends of specific location
*/
std::vector<std::vector<double>> getAisleEnds(const int loc_idx,
  const std::vector<std::vector<double>>& loc_mat,
  const std::vector<double>& wh_params);  // [w_cross_aisle, loc_wy, num_locs]

#endif  // WAREHOUSE_HPP
