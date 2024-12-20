/**
* @file
* @version 1.0
*
* @section DESCRIPTION
* Fatigue and rest models and their integration given picking tasks
* for fatigue level evolution and effect on performance.
*/
#ifndef FATIGUE_HPP
#define FATIGUE_HPP

#include "warehouse.hpp"
#include "routing.hpp"
#include <cmath>
#include <utility>
#include <list>
#include <vector>
#include <iostream>
#include <utility>

/**
* @brief Fatigue fractional increase in task time given a starting fatigue level.
* @details Select the fatigue function via the function_id parameter.
*          A = exponential increase
* @param function_id Function identifier
* @param Fprev Fatigue level before the task
* @param time Task time
* @param penibility Task penibility
* @return Fatigue level after the task
*/
double computeTaskFatigue(const char function_id,
                          const double Fprev, const double time,
                          const double penibility);

/**
* @brief Modified task time given a fatigue level
* @details Select the performance function via the function_id parameter.
*         A = logarithmic increase
* @param function_id Function identifier
* @param time_th Task time (theoretical)
* @param fatigue_level Fatigue level before the task
* @param correction_factor Correction factor for the task time
* @return Modified task time
*/
double computePickTime(const char function_id,
                       const double time_th, const double fatigue_level,
                       const double correction_factor);

/**
 * @brief Residual fatigue after tau time
 * @details Select the recovery function via the function_id parameter.
 *         A = exponential decrease
 * @param function_id Function identifier
 * @param Fprev Fatigue level before rest
 * @param tau Time of rest
 * @param recovery_rate Recovery rate
 * @return Fatigue level after rest (residual fatigue)
 */
double computeRecoveryLevel(const char function_id,
                            const double Fprev, const double tau,
                            const double recovery_rate);

/**
 * @brief Time for active rest to reach fatigue level Mth at the end of the task
 * @details Select the recovery function via the function_id parameter.
 *        A = exponential decrease
 * @param function_id Function identifier
 * @param Rprev Previous fatigue level at the begin of the pick
 * @param Fprev Previous fatigue level at the end of the pick (> Mth)
 * @param Mth Fatigue threshold (wich is Fnew at the end of the pick)
 * @param rest_rate Rest rate
 * @return Time for task with active rest time and related Rnew
 */
double computeRestTime(const char function_idRest, const char function_idTime,
                       const double Rprev, const double Fprev,
                       const double th_time, const double corr_fact,
                       const double pen,
                       const double Mth, const double rest_rate,
                       const double eps = 0.0001);

/**
 * @brief Get task time (possibly w/ rest) and Fnew for a task
 * @details Compute task time modified by fatigue and rest with travel recovery
*/
std::pair<double, double> computePositionalTimeFatigue(
  int prev_loc, double time_to_curr, double Fprev,
  const double th_time, const double pen,
  const std::vector<char> function_idx, const double correction_factor,
  const double rec_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level);

/**
 * @brief Returns route performance and final fatigue level
 * @details Compute route performance and final fatigue level given a batch.
 *          Avoids saving the full route and fatigue data for each task.
*/
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
  double *Favg = nullptr);



/**
 * @brief Fatigue level evolution given a route for a specific worker
 * @details Compute pairs of task time and fatigue level for each task 
 * (including rests/passive recovery). ONLY FOR FULL DESCRIPTION PURPOSES
 * @param route_w_times sequence of tasks with their travel times between picks
 * @param function_ids vector of function identifiers for fatigue, performance and recovery
 * @param pen_mat matrix of penibilities for each task
 * @param time_mat matrix of task times for each task
 * @param worker_id worker identifier
 * @param Fstart starting fatigue level
 * @param correction_factor correction factor for task times
 * @param recovery_rate recovery rate
 * @param rest_rate rest rate
 * @param fatigue_threshold fatigue threshold that triggers rest (before it is reached)
 * @param recovery_level fatigue level after rest
 * @return pair of total task time and list of pairs of task time and fatigue level
 */
std::pair<double, std::list<double>> computeFTevolution(
  const std::list<std::vector<double>>& route_w_times,
  const std::vector<char> function_ids,
  const std::vector<std::vector<double>>& pen_mat,
  const std::vector<std::vector<double>>& time_mat,
  const int worker_id,
  const double Fstart, const double correction_factor,
  const double recovery_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level);

/**
 * @brief Time components per task given a route for a specific worker
 * @details Useful for checking where task time comes from (pick action, travel, rest).
 *          Used in Python plots.
 *          Same parameters as computeFTevolution.
 *          ONLY FOR FULL DESCRIPTION PURPOSES OF A SOLUTION
 * @return vector task time components for each task in a route
 */
std::vector<std::vector<double>> computeFTtimesPerTask(
  const std::list<std::vector<double>>& route_w_times,
  const std::vector<char> function_ids,
  const std::vector<std::vector<double>>& pen_mat,
  const std::vector<std::vector<double>>& time_mat,
  const int worker_id,
  const double Fstart, const double correction_factor,
  const double recovery_rate, const double rest_rate,
  const double fatigue_threshold, const double recovery_level);

#endif  // FATIGUE_HPP
