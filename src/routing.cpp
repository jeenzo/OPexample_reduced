#include "routing.hpp"
#include "warehouse.hpp"


double manhattanDistance(double x1, double y1, double x2, double y2) {
  return std::abs(x1 - x2) + std::abs(y1 - y2);
}

std::list<std::vector<double>> sShapeWithTimeVector(
  const std::vector<int>& order_lines,
  const std::vector<std::vector<double>>& loc_mat,
  const std::vector<double>& wh_params,
  const bool get_route_coords) {
  // THIS FUNC NEEDS TO BE CONSISTENT WITH computeSshapePerformance()
  /* for getting route coords */
  std::vector<std::vector<double>> route_coords;
  // Sort order_lines by x value, then by y value
  std::vector<int> order_lines_sorted(order_lines);
  std::sort(order_lines_sorted.begin(), order_lines_sorted.end(),
          [&loc_mat](int i, int j) {
            return loc_mat[i][1] < loc_mat[j][1] ||
                   (loc_mat[i][1] == loc_mat[j][1] &&
                    loc_mat[i][2] < loc_mat[j][2]);
          });
  // Roodbergen & de Koster (2001)
  std::vector<std::vector<double>> route = {{0, 0.0}};
  double vel = 3;
  double time_to_i;
  //  > Step 1
  double x_leftmost = loc_mat[order_lines_sorted[0]][1];
  int furthest_block = -1;
  for (int i = 0; i < order_lines_sorted.size(); i++) {
    if (loc_mat[order_lines_sorted[i]][3] > furthest_block) {
      furthest_block = loc_mat[order_lines_sorted[i]][3];
    }
  }
  //  > Step 2+3
  // Move to front of furtherst block from leftmost aisle
  // = pick every order line with x = x_leftmost and block_idx < furthest_block
  for (int i = 0; i < order_lines_sorted.size(); i++) {
    if (loc_mat[order_lines_sorted[i]][3] == furthest_block) break;
    if (loc_mat[order_lines_sorted[i]][1] == x_leftmost
        && loc_mat[order_lines_sorted[i]][3] < furthest_block) {
      time_to_i = 1 / vel * manhattanDistance(loc_mat[route.back()[0]][1],
                                      loc_mat[route.back()[0]][2],
                                      loc_mat[order_lines_sorted[i]][1],
                                      loc_mat[order_lines_sorted[i]][2]);
      route.push_back({static_cast<double>(order_lines_sorted[i]), time_to_i});
    }
  }
  /* for getting route coords */
  if (get_route_coords) {
    // First location is depot
    route_coords.push_back({loc_mat[0][1], loc_mat[0][2]});
    // Second location is the aisle end of the first order line nearest to depot
    std::vector<std::vector<double>> aisle_ends = getAisleEnds(route[1][0], loc_mat, wh_params);
    // Append the aisle end with y more similar to depot
    if (std::abs(aisle_ends[0][1] - loc_mat[0][2]) < std::abs(aisle_ends[1][1] - loc_mat[0][2])) {
      route_coords.push_back({aisle_ends[0][0], aisle_ends[0][1]});
    } else {
      route_coords.push_back({aisle_ends[1][0], aisle_ends[1][1]});
    }
    // Append the rest of the route, if any
    if (route.size() > 2) {
      for (int i = 2; i < route.size(); i++) {
        route_coords.push_back({loc_mat[route[i][0]][1], loc_mat[route[i][0]][2]});
      }
    }
  }

  // > Step 4+5+6+7
  // Group order lines in furtherst block by x value (ascending)
  std::vector<std::vector<int>> last_block_orders;
  bool first = true;
  for (int i = 0; i < order_lines_sorted.size(); i++) {
    if (loc_mat[order_lines_sorted[i]][3] == furthest_block && first) {
      last_block_orders.push_back({order_lines_sorted[i]});
      first = false;
    } else if (loc_mat[order_lines_sorted[i]][3] == furthest_block
      && loc_mat[order_lines_sorted[i]][1] == loc_mat[last_block_orders.back()[0]][1]) {
      last_block_orders.back().push_back(order_lines_sorted[i]);
    } else if (loc_mat[order_lines_sorted[i]][3] == furthest_block) {
      last_block_orders.push_back({order_lines_sorted[i]});
    }
  }
  // Append the order lines in the last block to route
  for (int i = 0; i < last_block_orders.size(); i++) {
    // Invert order for odd i
    int front_or_end = 0;
    if (i % 2 == 1) {
      std::reverse(last_block_orders[i].begin(), last_block_orders[i].end());
      front_or_end = 1;
    }
    for (int j = 0; j < last_block_orders[i].size(); j++) {
      if (j == 0) {
        time_to_i = 1 / vel * (
            manhattanDistance(loc_mat[route.back()[0]][1],
                              loc_mat[route.back()[0]][2],
                              getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])
            + manhattanDistance(getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                                loc_mat[last_block_orders[i][j]][1],
                                loc_mat[last_block_orders[i][j]][2]));
          /* for getting route coords */
          if (get_route_coords) {
            // Append coords of aisle end and order j
            route_coords.push_back({getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                    getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][1]});
            route_coords.push_back({loc_mat[last_block_orders[i][j]][1], loc_mat[last_block_orders[i][j]][2]});
          }

      } else {
        time_to_i = 1 / vel * (manhattanDistance(loc_mat[route.back()[0]][1],
                                      loc_mat[route.back()[0]][2],
                                      loc_mat[last_block_orders[i][j]][1],
                                      loc_mat[last_block_orders[i][j]][2]));
        /* for getting route coords */
        if (get_route_coords) {
          // Append coords of order j
          route_coords.push_back({loc_mat[last_block_orders[i][j]][1], loc_mat[last_block_orders[i][j]][2]});
        }
      }
      route.push_back({static_cast<double>(last_block_orders[i][j]), time_to_i});
    }
  }
  last_block_orders.clear();
  // > Step 8
  if (furthest_block == 0){
    // Append depot
    /* for getting route coords */
    if (get_route_coords) route_coords.push_back({loc_mat[0][1], loc_mat[0][2]});

    route.push_back({0, manhattanDistance(loc_mat[route.back()[0]][1],
                                          loc_mat[route.back()[0]][2],
                                          loc_mat[0][1],
                                          loc_mat[0][2])});
    order_lines_sorted.clear();
    if (get_route_coords) {
      // To list with make_move_iterator
      std::list<std::vector<double>> route_coords_list{std::make_move_iterator(route_coords.begin()),
                                                       std::make_move_iterator(route_coords.end())};
      return route_coords_list;
    }
    // To list with make_move_iterator
    std::list<std::vector<double>> route_list{std::make_move_iterator(route.begin()),
                                              std::make_move_iterator(route.end())};
    return route_list;
  }
  // Repeat for very block until the first
  int curr_block = furthest_block - 1;
  while (curr_block > 0) {
    // Group order lines in current block by x value (ascending)
    std::vector<std::vector<int>> curr_block_orders;
    bool first = true;
    for (int i = 0; i < order_lines_sorted.size(); i++) {
      // Skip all orders in first aisle
      if (loc_mat[order_lines_sorted[i]][1] == x_leftmost) continue;
      if (loc_mat[order_lines_sorted[i]][3] == curr_block && first) {
        curr_block_orders.push_back({order_lines_sorted[i]});
        first = false;
      } else if (loc_mat[order_lines_sorted[i]][3] == curr_block
        && loc_mat[order_lines_sorted[i]][1] == loc_mat[curr_block_orders.back()[0]][1]) {
        curr_block_orders.back().push_back(order_lines_sorted[i]);
      } else if (loc_mat[order_lines_sorted[i]][3] == curr_block) {
        curr_block_orders.push_back({order_lines_sorted[i]});
      }
    }
    // Append the order lines in the current block to route
    for (int i = 0; i < curr_block_orders.size(); i++) {
      // Invert order for odd i
      int front_or_end = 0;
      if (i % 2 == 0) {
        std::reverse(curr_block_orders[i].begin(), curr_block_orders[i].end());
        front_or_end = 1;
      }
      for (int j = 0; j < curr_block_orders[i].size(); j++) {
        if (j == 0) {
          time_to_i = 1 / vel * (
            manhattanDistance(loc_mat[route.back()[0]][1],
                              loc_mat[route.back()[0]][2],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])

            + manhattanDistance(getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                                loc_mat[curr_block_orders[i][j]][1],
                                loc_mat[curr_block_orders[i][j]][2]));
          /* for getting route coords */
          if (get_route_coords) {
            // Append coords of aisle end and order j
            route_coords.push_back({getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                    getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1]});
            route_coords.push_back({loc_mat[curr_block_orders[i][j]][1], loc_mat[curr_block_orders[i][j]][2]});
          }
        } else {
          time_to_i = 1 / vel * (manhattanDistance(loc_mat[route.back()[0]][1],
                                        loc_mat[route.back()[0]][2],
                                        loc_mat[curr_block_orders[i][j]][1],
                                        loc_mat[curr_block_orders[i][j]][2]));
          /* for getting route coords */
          if (get_route_coords) {
            // Append coords of order j
            route_coords.push_back({loc_mat[curr_block_orders[i][j]][1], loc_mat[curr_block_orders[i][j]][2]});
          }
        }
        route.push_back({static_cast<double>(curr_block_orders[i][j]), time_to_i});
      }
    }
    curr_block--;
    curr_block_orders.clear();
  }
  // First block
  std::vector<std::vector<int>> curr_block_orders;
  first = true;
  for (int i = order_lines_sorted.size() - 1; i >= 0; i--) {
    // Skip all orders in first aisle
    if (loc_mat[order_lines_sorted[i]][1] == x_leftmost) continue;
    if (loc_mat[order_lines_sorted[i]][3] == curr_block && first) {
      curr_block_orders.push_back({order_lines_sorted[i]});
      first = false;
    } else if (loc_mat[order_lines_sorted[i]][3] == curr_block
      && loc_mat[order_lines_sorted[i]][1] == loc_mat[curr_block_orders.back()[0]][1]) {
      curr_block_orders.back().push_back(order_lines_sorted[i]);
    } else if (loc_mat[order_lines_sorted[i]][3] == curr_block) {
      curr_block_orders.push_back({order_lines_sorted[i]});
    }
  }
  // Append the order lines in the current block to route
  for (int i = 0; i < curr_block_orders.size(); i++) {
    // Invert order for odd i
    int front_or_end = 1;
    if (i % 2 == 1) {
      std::reverse(curr_block_orders[i].begin(), curr_block_orders[i].end());
      front_or_end = 0;
    }
    for (int j = 0; j < curr_block_orders[i].size(); j++) {
      if (j == 0) {
        time_to_i = 1 / vel * (
          manhattanDistance(loc_mat[route.back()[0]][1],
                            loc_mat[route.back()[0]][2],
                            getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                            getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])
          + manhattanDistance(getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                              loc_mat[curr_block_orders[i][j]][1],
                              loc_mat[curr_block_orders[i][j]][2]));
        /* for getting route coords */
        if (get_route_coords) {
          // Append coords of aisle end and order j
          route_coords.push_back({getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                  getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1]});
          route_coords.push_back({loc_mat[curr_block_orders[i][j]][1], loc_mat[curr_block_orders[i][j]][2]});
        }
      } else {
        time_to_i = 1 / vel * (manhattanDistance(loc_mat[route.back()[0]][1],
                                      loc_mat[route.back()[0]][2],
                                      loc_mat[curr_block_orders[i][j]][1],
                                      loc_mat[curr_block_orders[i][j]][2]));
        /* for getting route coords */
        if (get_route_coords) {
          // Append coords of order j
          route_coords.push_back({loc_mat[curr_block_orders[i][j]][1], loc_mat[curr_block_orders[i][j]][2]});
        }
      }
      route.push_back({static_cast<double>(curr_block_orders[i][j]), time_to_i});
    }
    curr_block_orders.clear();
  }
  // Append depot
  /* for getting route coords */
  if (get_route_coords) route_coords.push_back({loc_mat[0][1], loc_mat[0][2]});

  route.push_back({0, manhattanDistance(loc_mat[route.back()[0]][1],
                                        loc_mat[route.back()[0]][2],
                                        loc_mat[0][1],
                                        loc_mat[0][2])});
  order_lines_sorted.clear();
  if (get_route_coords) {
    // To list with make_move_iterator
    std::list<std::vector<double>> route_coords_list{std::make_move_iterator(route_coords.begin()),
                                                     std::make_move_iterator(route_coords.end())};
    return route_coords_list;
  }
  // To list with make_move_iterator
  std::list<std::vector<double>> route_list{std::make_move_iterator(route.begin()),
                                            std::make_move_iterator(route.end())};
  return route_list;
}
