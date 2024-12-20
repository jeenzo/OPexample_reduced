#include "fatigue.hpp"
#include <tuple>

double computeTaskFatigue(const char function_id,
                          const double Fprev, const double time,
                          const double penibility) {
  double fractional_increase = 0;
  switch (function_id) {
    case 'A':
      fractional_increase = (1 - exp(-penibility * time)) * (1 - Fprev);   // ## TODO try to find faster exp alternative
      break;
    default:
      std::cout << " > Warning 'computeTaskFatigue()': ";
      std::cout << "Function ID not recognized" << std::endl;
  }
  return Fprev + fractional_increase;
}

double computePickTime(const char function_id,
                       const double time_th, const double fatigue_level,
                       const double correction_factor) {
  double time_increase = 0;
  switch (function_id) {
    case 'A':
      time_increase = time_th * correction_factor * std::log(1 + fatigue_level);  // ## TODO try to find faster log alternative
      break;
    default:
      std::cout << " > Warning 'computePickTime()': ";
      std::cout << "Function ID not recognized" << std::endl;
  }
  return time_th + time_increase;
}

double computeRecoveryLevel(const char function_id,
                            const double Fprev, const double tau,
                            const double recovery_rate) {
  switch (function_id) {
    case 'A':
      return Fprev * exp(-recovery_rate * tau);    // ## TODO try to find faster exp alternative
    default:
      std::cout << " > Warning 'computeRecoveryLevel()': ";
      std::cout << "Function ID not recognized" << std::endl;
  }
  
  return Fprev;
}

// New approach with UB + bisection
double computeRestTime(const char function_idRest, const char function_idTime,
                       const double Rprev, const double Fprev,
                       const double th_time, const double corr_fact,
                       const double pen,
                       const double Mth, const double rest_rate,
                       const double eps) {
  // Bisection
  double lb = 0;
  double ub = Rprev;
  double Rnew = (lb + ub) / 2;
  double Fnew = Fprev;
  double task_time;
  if (Fnew < Mth + eps && Fnew > Mth - eps) return 0;
  while (Fnew > Mth + eps || Fnew < Mth - eps) {
    task_time = computePickTime(function_idTime, th_time, Rnew, corr_fact);
    Fnew = computeTaskFatigue(function_idRest, Rnew, task_time, pen);
    if (Fnew < Mth) {
      lb = Rnew;
    } else {
      ub = Rnew;
    }
    Rnew = (lb + ub) / 2;
  }
  // From Rnew to rest time tau = Log(Rprev/Rnew)/rest_rate
  if (function_idRest == 'A') return std::log(Rprev / Rnew) / rest_rate;  // ## TODO try to find faster log alternative
  return 0;
}


std::pair<double, double> computePositionalTimeFatigue(
  int prev_loc, double time_to_curr, double Fprev,
  const double th_time, const double pen,
  const std::vector<char> function_idx, const double correction_factor,
  const double rec_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level) {
  // Compute new fatigue level and modified task time,
  // considering rest if threhsould would be reached


  double Rnew = computeRecoveryLevel(function_idx[2], Fprev, time_to_curr, rec_rate);
  double task_time = computePickTime(function_idx[1], th_time, Rnew, correction_factor);
  double rest_time = 0;
  if (computeTaskFatigue(function_idx[0], Rnew, task_time, pen) > fatigue_threshold) {
    rest_time = computeRestTime(function_idx[3], function_idx[1],
                                Rnew, computeTaskFatigue(function_idx[0], Rnew, task_time, pen),
                                th_time, correction_factor, pen,
                                fatigue_threshold, rest_rate);
    Rnew = computeRecoveryLevel(function_idx[3], Rnew, rest_time, rest_rate);
    task_time = computePickTime(function_idx[1], th_time, Rnew, correction_factor);
  }
  // Compute new fatigue level Fnew
  Rnew = computeTaskFatigue(function_idx[0], Rnew, task_time, pen);
  // return std::make_pair(0, 0);
  // Print [loc, F, time]
  // std::cout << prev_loc << ", " << Rnew << ", " << time_to_curr << "," << task_time << std::endl;
  return std::make_pair(task_time + rest_time, Rnew);
}

std::pair<double, double> computeSshapePerformance(
                        const std::vector<int>& order_lines,
                        const std::vector<std::vector<double>>& loc_mat,
                        const std::vector<double>& wh_params,
                        const std::vector<std::vector<double>>& pen_mat,
                        const std::vector<std::vector<double>>& time_mat,
                        const std::vector<char> function_idx,
                        const int worker_id,
                        const double Fstart, const double correction_factor,
                        const double recovery_rate, const double rest_rate,
                        const double fatigue_threshold, const double recovery_level,
                        double *Favg) {
  // Parametrize later on
  double vel = 3;  // ~11km/h https://www.still.co.uk/forklift-trucks/new-forklifts/order-pickers/opx.html
  // Performance variables
  double Fnew = Fstart;
  double tot_time = 0;
  // Params that reset
  int prev_node = 0;
  double time_to_i;
  double time_i;
  // Sort order_lines by x value  ## TODO do this without copying
  std::vector<int> order_lines_sorted(order_lines);
  std::sort(order_lines_sorted.begin(), order_lines_sorted.end(),
          [&loc_mat](int i, int j) {
            return loc_mat[i][1] < loc_mat[j][1] ||
                   (loc_mat[i][1] == loc_mat[j][1] &&
                    loc_mat[i][2] < loc_mat[j][2]);
          });
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
      // NEW ROUTE NODE
      time_to_i = 1 / vel * manhattanDistance(loc_mat[prev_node][1],
                                      loc_mat[prev_node][2],
                                      loc_mat[order_lines_sorted[i]][1],
                                      loc_mat[order_lines_sorted[i]][2]);
      std::tie(time_i, Fnew) = computePositionalTimeFatigue(
                        order_lines_sorted[i], time_to_i, Fnew,
                        time_mat[order_lines_sorted[i]][worker_id], pen_mat[order_lines_sorted[i]][worker_id],
                        function_idx, correction_factor, recovery_rate, rest_rate,
                        fatigue_threshold, recovery_level);
      tot_time += time_i + time_to_i;
      if (Favg != nullptr) *Favg += Fnew;
      prev_node = order_lines_sorted[i];
    }
  }

  // > Step 4+5+6+7
  // Group order lines in furtherst block by x value (ascending)  ## TODO do this without copying
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
      // NEW ROUTE NODE
      if (j == 0) {
        time_to_i = 1 / vel * (
            manhattanDistance(loc_mat[prev_node][1],
                              loc_mat[prev_node][2],
                              getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])
            + manhattanDistance(getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                getAisleEnds(last_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                                loc_mat[last_block_orders[i][j]][1],
                                loc_mat[last_block_orders[i][j]][2])
            );
      } else {
        time_to_i = 1 / vel * (manhattanDistance(loc_mat[prev_node][1],
                                      loc_mat[prev_node][2],
                                      loc_mat[last_block_orders[i][j]][1],
                                      loc_mat[last_block_orders[i][j]][2]));
      }
      std::tie(time_i, Fnew) = computePositionalTimeFatigue(last_block_orders[i][j], time_to_i, Fnew,
                        time_mat[last_block_orders[i][j]][worker_id], pen_mat[last_block_orders[i][j]][worker_id],
                        function_idx, correction_factor, recovery_rate, rest_rate,
                        fatigue_threshold, recovery_level);
      tot_time += time_i + time_to_i;
      if (Favg != nullptr) *Favg += Fnew;
      prev_node = last_block_orders[i][j];
    }
  }
  last_block_orders.clear();
  // > Step 8
  if (furthest_block == 0){
    // FINAL ROUTE NODE - Append depot
    time_to_i = 1 / vel * manhattanDistance(loc_mat[prev_node][1],
                                            loc_mat[prev_node][2],
                                            loc_mat[0][1],
                                            loc_mat[0][2]);
    std::tie(time_i, Fnew) = computePositionalTimeFatigue(0, time_to_i, Fnew,
                          time_mat[0][worker_id], pen_mat[0][worker_id],
                          function_idx, correction_factor, recovery_rate, rest_rate,
                          fatigue_threshold, recovery_level);
    tot_time += time_i + time_to_i;
    if (Favg != nullptr) *Favg += Fnew;
    // prev_node = 0;
    // Clear and return
    order_lines_sorted.clear();
    return std::make_pair(tot_time, Fnew);
  }
  // Repeat for very block until the first
  int curr_block = furthest_block - 1;
  while (curr_block > 0) {
    // Group order lines in current block by x value (ascending)  ## TODO do this without copy
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
        // NEW ROUTE NODE
        if (j == 0) {
          time_to_i = 1 / vel * (
            manhattanDistance(loc_mat[prev_node][1],
                              loc_mat[prev_node][2],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])

            + manhattanDistance(getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                                getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                                loc_mat[curr_block_orders[i][j]][1],
                                loc_mat[curr_block_orders[i][j]][2]));
        } else {
          time_to_i = 1 / vel * (manhattanDistance(loc_mat[prev_node][1],
                                        loc_mat[prev_node][2],
                                        loc_mat[curr_block_orders[i][j]][1],
                                        loc_mat[curr_block_orders[i][j]][2]));
        }
        std::tie(time_i, Fnew) = computePositionalTimeFatigue(curr_block_orders[i][j], time_to_i, Fnew,
                          time_mat[curr_block_orders[i][j]][worker_id], pen_mat[curr_block_orders[i][j]][worker_id],
                          function_idx, correction_factor, recovery_rate, rest_rate,
                          fatigue_threshold, recovery_level);
        tot_time += time_i + time_to_i;
        if (Favg != nullptr) *Favg += Fnew;
        prev_node = curr_block_orders[i][j];
      }
    }
    curr_block--;
    curr_block_orders.clear();
  }
  // First block  ## TODO do this without copying
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
      // NEW ROUTE NODE
      if (j == 0) {
        time_to_i = 1 / vel * (
          manhattanDistance(loc_mat[prev_node][1],
                            loc_mat[prev_node][2],
                            getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                            getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1])
          + manhattanDistance(getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][0],
                              getAisleEnds(curr_block_orders[i][j], loc_mat, wh_params)[front_or_end][1],
                              loc_mat[curr_block_orders[i][j]][1],
                              loc_mat[curr_block_orders[i][j]][2]));
      } else {
        time_to_i = 1 / vel * (manhattanDistance(loc_mat[prev_node][1],
                                      loc_mat[prev_node][2],
                                      loc_mat[curr_block_orders[i][j]][1],
                                      loc_mat[curr_block_orders[i][j]][2]));
      }
      std::tie(time_i, Fnew) = computePositionalTimeFatigue(curr_block_orders[i][j], time_to_i, Fnew,
                        time_mat[curr_block_orders[i][j]][worker_id], pen_mat[curr_block_orders[i][j]][worker_id],
                        function_idx, correction_factor, recovery_rate, rest_rate,
                        fatigue_threshold, recovery_level);
      tot_time += time_i + time_to_i;
      if (Favg != nullptr) *Favg += Fnew;
      prev_node = curr_block_orders[i][j];
    }
    curr_block_orders.clear();
  }
  // FINAL ROUTE NODE - Append depot
  time_to_i = 1 / vel * manhattanDistance(loc_mat[prev_node][1],
                                          loc_mat[prev_node][2],
                                          loc_mat[0][1],
                                          loc_mat[0][2]);
  std::tie(time_i, Fnew) = computePositionalTimeFatigue(0, time_to_i, Fnew,
                        time_mat[0][worker_id], pen_mat[0][worker_id],
                        function_idx, correction_factor, recovery_rate, rest_rate,
                        fatigue_threshold, recovery_level);
  tot_time += time_i + time_to_i;
  if (Favg != nullptr) *Favg += Fnew;
  // prev_node = 0;
  if (Favg != nullptr) *Favg /= (order_lines.size()+2);
  // if (Favg != nullptr) std::cout << *Favg << std::endl;
  // Clear and return
  order_lines_sorted.clear();
  return std::make_pair(tot_time, Fnew);
}

std::pair<double, std::list<double>> computeFTevolution(
  const std::list<std::vector<double>>& route_w_times,
  const std::vector<char> function_ids,
  const std::vector<std::vector<double>>& pen_mat,
  const std::vector<std::vector<double>>& time_mat,
  const int worker_id,
  const double Fstart, const double correction_factor,
  const double recovery_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level) {
  // Function_ids are in the order of [Fatigue, PickTime, Recovery, Rest]
  double Fnew = Fstart;
  double Rnew;
  double tnew;
  double task_time;
  double rest_time;
  // Iterate for each task in route
  std::list<double> fatigue_levels;
  double tot_time = 0;
  for (auto task : route_w_times) {
    // Compute fatigue level
    tnew = task[1];
    Rnew = computeRecoveryLevel(function_ids[2], Fnew, tnew, recovery_rate);
    // Pick time
    task_time = computePickTime(function_ids[1],
                                time_mat[task[0]][worker_id],
                                Rnew, correction_factor);
    // Mandatory rest (before task) if the execution of this task would lead to Fnew > fatigue_threshold
    if (computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]) > fatigue_threshold) {
      rest_time = computeRestTime(function_ids[3], function_ids[1],
                                Rnew, computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]),
                                time_mat[task[0]][worker_id], correction_factor, pen_mat[task[0]][worker_id],
                                fatigue_threshold, rest_rate);
      Rnew = computeRecoveryLevel(function_ids[3], Rnew, rest_time, rest_rate);
      tnew += rest_time;
      task_time = computePickTime(function_ids[1],
                                time_mat[task[0]][worker_id],
                                Rnew, correction_factor);
    }
    tnew += task_time;
    // Update fatigue level
    Fnew = computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]);
    // Push fatigue level
    fatigue_levels.push_back(Fnew);
    // Update total time
    tot_time += tnew;
  }
  return std::make_pair(tot_time, fatigue_levels);
}

std::vector<std::vector<double>> computeFTtimesPerTask(
  const std::list<std::vector<double>>& route_w_times,
  const std::vector<char> function_ids,
  const std::vector<std::vector<double>>& pen_mat,
  const std::vector<std::vector<double>>& time_mat,
  const int worker_id,
  const double Fstart, const double correction_factor,
  const double recovery_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level) {
  // Return [travel_time, task_time_th, task_time_delta_fatigue, rest_time] for each task in a route
  // Code for computing fatigue increment follows computeFTevolution()
  double travel_time = 0;
  double task_time_th = 0;
  double task_time_delta_fatigue = 0;
  double rest_time = 0;
  std::vector<std::vector<double>> times_per_task;

  double Fnew = Fstart;
  double Rnew;
  double tnew;
  double task_time;
  // Iterate for each task in route
  std::list<double> fatigue_levels;
  for (auto task : route_w_times) {
    // Compute fatigue level
    rest_time = 0;
    tnew = task[1];
    travel_time = tnew;
    Rnew = computeRecoveryLevel(function_ids[2], Fnew, tnew, recovery_rate);
    // Pick time
    task_time_th = time_mat[task[0]][worker_id];
    task_time = computePickTime(function_ids[1],
                                time_mat[task[0]][worker_id],
                                Rnew, correction_factor);
    task_time_delta_fatigue = task_time - task_time_th;
    // Mandatory rest?
    if (computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]) > fatigue_threshold) {
      rest_time = computeRestTime(function_ids[3], function_ids[1],
                                Rnew, computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]),
                                time_mat[task[0]][worker_id], correction_factor, pen_mat[task[0]][worker_id],
                                fatigue_threshold, rest_rate);
      tnew += rest_time;
      Rnew = computeRecoveryLevel(function_ids[3], Rnew, rest_time, rest_rate);
      task_time = computePickTime(function_ids[1],
                                time_mat[task[0]][worker_id],
                                Rnew, correction_factor);
    }
    tnew += task_time;
    // Update fatigue level
    Fnew = computeTaskFatigue(function_ids[0], Rnew, task_time,
                              pen_mat[task[0]][worker_id]);
    // Push fatigue level
    fatigue_levels.push_back(Fnew);
    // Push times
    times_per_task.push_back({travel_time, task_time_th, task_time_delta_fatigue, rest_time});
  }
  return times_per_task;
}
