/**
* @file
* @version 0.0
*
* @section DESCRIPTION
* ALNS implementation with operators from operators.hpp.
*/
#ifndef ALNS_HPP
#define ALNS_HPP

#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <numeric>
#include "problem_instance.hpp"
#include "operators.hpp"


/**
* @brief ALNS class to act on a specific PI with index of operators to be used
*
* @details Assumes that PI is already initialized, with batches assigned to workers,
* and their related routes and fatigue computed (computeTotalPerformance() already
* called on feasible initial solution).
* 
* Best and current solutions are saved as workers. New solution is the current PI workers
*/
class ALNS {
 public:
  ALNS(PI * ptr, int nr_operators, std::vector<int> idx_operators);
  ~ALNS(void);
  /**
   * @brief Get current weights of the operators
   * @return Vector of weights
   */
  std::vector<double> getWeights() { return weights_; }
  /**
   * @brief Roulette wheel selection
   * @return Index of operator to execute
   */
  int randomOperator();
  /**
   * @brief Execute one step of ALNS
   * @param idx_op Index of operator to execute
   * @param Q Max neighbourhood size
   */
  void executeStep(int idx_op, int Q = 1);
  /**
   * @brief Run ALNS optimization
   * @details Every segement weights are updated. SA dictates soluion acceptance.
   * @param n_segments Number of segments to execute
   * @param iter_per_seg Number of iterations per segment
   * @param sigmas Vector of weights for updating operator scores. 
   *        sigmas[0] for better new solution,
   *        sigmas[1] for worse new solution accepted by SA, 
   *        sigmas[2] for worse new solution rejected by SA
   * @param r rate for updating weights
   * @param cool_rate cooling rate for SA
   * @param worsening_factor (0<w<1) sets Tstart to have a probability of 50%
   *        to accept a solution that is w% worse than the current one
   * @param Qmax Max neighbourhood size
   * @param track_segments If true, save the best solution performance for each segment
   *        and operator weights to file
   * @param folder_name Folder name to save the file for tracking segments
   */
  void run(int n_segments, int iter_per_seg, std::vector<double> sigmas,
           double r, double cool_rate, double worsening_factor,
           int Qmax, int reset_at, int overwrite_for, double shakeT, double shakec,
           bool track_segments, bool track_iter, std::string& folder_name);
  /**
   * @brief Update weights of operators
   * @param r rate for updating weights
   */
  void updateWeights(double r);
  /**
   * @brief Print best, current and new solutions and related batches
   */
  void printSolutions();
  /**
   * @brief Update PI current solution to best solution
   */
  void updateToBest() { PIptr_->updateWorkers(best_sol_); }

  void saveOperatorTotParams(std::string& folder_name);

  // For comparison on small instances. Enumerate all possible solutions (only for single Worker)
  void bruteforce();

 private:
  PI * PIptr_;
  std::vector<int> idx_operators_;
  Operators op_;
  // Solutions are the workers
  std::vector<Worker> best_sol_;
  double best_perf_;
  std::vector<Worker> curr_sol_;
  double curr_perf_;
  double new_perf_;
  // Params
  std::vector<double> weights_;
  std::vector<int> usage_nrs_;
  std::vector<double> scores_;
  // Track total performance
  std::vector<double> tot_run_times_;
  std::vector<double> tot_usage_nrs_;
  std::vector<double> tot_improvement_;
};


#endif  // ALNS_HPP
