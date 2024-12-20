/**
* @file
* @version 1.0
*
* @section DESCRIPTION
* Operators for the ALNS that act on a specific PI. All operators produce a feasible solution.
*/
# ifndef OPERATORS_HPP
# define OPERATORS_HPP

#include <random>
#include <tuple>
#include <iostream>
#include "problem_instance.hpp"

/**
 * @brief Operators class that takes the current problem instance to modify it at a Worker level (batches).
 * @details The operators are:
 *         - random_greedy: remove random order, insert in best position
 *         - random_random: remove random order, insert in random position
 *         - swap: swap 2 orders in different batches
 *         - swap_batches: swap 2 batches between 2 pickers
 *         - worst_greedy: remove worst order, insert in best position
 *         - worst_random: remove worst order, insert in random position
 *         - (ToDo) empty_greedy: remove all batches from a picker, insert orders in best position
 */
class Operators {
 public:
  explicit Operators(PI * ptr);
  ~Operators(void);

  // Constructor for declaration then initialization
  Operators() = default;

  /** 
   * @brief Randomly remove an order from a batch and insert it in the best position.
   * @details Can act on empty batches. Order may be reinserted in original position.
   * @param q Size of the neighbourhood.
   */
  void random_greedy(int q = 1); 
  /**
   * @brief Randomly remove an order from a batch and insert it in a random position.
   * @details Can act on empty batches. Order may be reinserted in original position.
   * @param q Size of the neighbourhood.
   */
  void random_random(int q = 1);
  /**
   * @brief Swap 2 orders in different batches.
   * @details Acts only on non-empty batches. Orders may be in the same batch.
   */
  void swap();
  /**
   * @brief Swap 2 batches between 2 pickers.
   * @details Acts only on non-empty batches. Batches may belong to the same Worker.
   */
  void swap_batches();
  /**
   * @brief Remove the worst order and insert it in the best position.
   * @details Can act on empty batches. Order may be reinserted in original position.
   * @param q Size of the neighbourhood.
   */
  void worst_greedy(int q = 1);
  /**
   * @brief Remove the worst order and insert it in a random position.
   * @details Can act on empty batches. Order may be reinserted in original position.
   * @param q Size of the neighbourhood.
   */
  void worst_random(int q = 1);
  /**
   * @brief Remove a random order from the batch with highest Fatigue peak and insert it in the best position.
  */
  void Fmax_greedy();
  /**
   * @brief Remove a random order from the batch with highest Fatigue peak and insert it in a random position.
  */
  void Fmax_random();
  /**
   * @brief Remove all orders from the batches of the worker with highest average fatigue and insert them in the best position.
  */
  void empty_greedy();
  /**
   * @brief Like Fmax_greedy but order is selected based on average penibility of its items.
  */
  void Fmax_greedy_w_pen();
  /**
   * @brief Like Fmax_random but order is selected based on average penibility of its items.
  */
  void Fmax_random_w_pen();
  // ... fatigue-based insertions
  /**
   * @brief Insertion is done in the batch that would result in the lowest average fatigue after insertion.
  */
  void random_Flow();
  void worst_Flow();
  void Fmax_Flow();
  void Fmax_Flow_w_pen();

  // Operators on batches
  void b_random_random();
  void b_random_greedy();
  void b_random_Fend();
  void b_worst_random();
  void b_worst_greedy();
  void b_worst_Fend();
  void b_Fmax_random();
  void b_Fmax_greedy();
  void b_Fmax_Fend();

  /**
   * @brief Execute the operator via index.
   * @details This function is used by the ALNS class to execute the operator.
   * 
   * @param idx_op Index of the operator to execute.
   *        0 is random_greedy, 1 is random_random, 2 is swap, 3 is swap_batches,
   *        4 is worst_greedy, 5 is worst_random.
   * @param q Size of the neighbourhood.
   */
  void execute(int idx_op, int q = 1);

 private:
  PI * PIptr_;
};


#endif  // OPERATORS_HPP
