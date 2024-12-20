/**
* @file
* @version 1.0
*
* @section DESCRIPTION
* Routing functions and heuristics.
*/
#ifndef ROUTING_HPP
#define ROUTING_HPP

#include <vector>
#include <list>
#include <algorithm>

/**
* @brief (Write...) Short description
* @param x1 x coordinate of first point
* @param y1 y coordinate of first point
* @param x2 x coordinate of second point
* @param y2 y coordinate of second point
*/
double manhattanDistance(double x1, double y1, double x2, double y2);

/**
* @brief Returns the route sequence and time needed to get from previous to current pick
  @details This is used to get full description of a batch on a route level
*/
std::list<std::vector<double>> sShapeWithTimeVector(
  const std::vector<int>& order_lines,
  const std::vector<std::vector<double>>& loc_mat,
  const std::vector<double>& wh_params,
  const bool get_route_coords = false);  // params are the ones needed for getAisleEnds
                                          // [w_cross_aisle, loc_wy, num_locs]

#endif  // ROUTING_HPP
