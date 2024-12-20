/**
* @file
* @version 1.0
*
* @section DESCRIPTION
* Read input data from files and save them to a problem-specific struct.
*/
#ifndef INPUTS_HPP
#define INPUTS_HPP

#include <string>
#include <vector>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>


/**
 * @brief Read warehouse data from .csv file
 * @param filename Name of the file
 * @return Vector of {num_aisles, num_blocks, num_locs, loc_wx, loc_wy, w_aisle, w_cross_aisle, x0, y0}
 */
std::vector<double> readWarehouseChar(const std::string& filename);

/**
 * @brief Read location matrix from .csv file
 * @param filename Name of the file
 * @return Vector of {loc_id, x, y, block_nr, aisle_side}
 */
std::vector<std::vector<double>> readLocationMatrix(const std::string& filename);

/**
 * @brief Read orders from .csv file
 * @param filename Name of the file
 * @return Vector of prod_id for each order (multiple qty order lines are repeated entries)
 */
std::vector<std::vector<int>> readOrdersFilePy(const std::string& filename);

/**
 * @brief Read any 2D matrix of doubles
 * @param filename Name of the file
 * @param header True if the file has a header (e.g., for worker data)
 */
std::vector<std::vector<double>> readMatrix(const std::string& filename,
                                            const bool header = false);

/**
 * @brief struct to store all input data
 * @param loc_mat Location matrix
 * @param order_lines Order lines
 * @param n_aisles Number of aisles
 * @param n_blocks Number of blocks
 * @param n_loc_per_aisle Number of locations per aisle side
 * @param N Number of orders
 * @param wx storage location dept
 * @param wy storage location width
 * @param w_aisle aisle width
 * @param w_cross cross aisle width
 * @param x0 x coordinate of the depot
 * @param y0 y coordinate of the depot
 * @param pen_mat Penibility matrix
 * @param time_mat Time matrix (theoretical)
 * @param worker_data Worker-specific data: Fstart,recovery_rate,rest_rate,time_correction_factor,Fth,Flow
 */
struct InputData {
  std::vector<std::vector<double>> loc_mat;
  std::vector<std::vector<int>> order_lines;
  int n_aisles, n_blocks, n_loc_per_aisle;
  int N;
  double wx, wy;
  double w_aisle, w_cross;
  double x0, y0;
  std::vector<std::vector<double>> pen_mat;
  std::vector<std::vector<double>> time_mat;
  std::vector<std::vector<double>> worker_data;
};

/**
 * @brief Read all input data from files
 * @param wh_file Warehouse file (both params and location matrix)
 * @param orders_file Orders file
 * @param pen_file Penibility file
 * @param time_file Task time file
 * @param worker_file Worker file
 * @return InputData struct
 */
InputData readInputs(const std::string& wh_file,
                     const std::string& orders_file,
                     const std::string& pen_file,
                     const std::string& time_file,
                     const std::string& worker_file);

/**
 * @brief Check if input data is valid
 * @details Perform several checks on input data to see if different files are consistent
 * @param data InputData struct
 * @param idx_workers Vector of worker indices considered in the problem
 * @return True if input data is valid, false otherwise
 */
bool inputValidator(InputData data, std::vector<int>& idx_workers);


/**
 * @brief Reads batches from file
 * @details File is generated from the Python notebook, 
 * which reads a .txt file with contents given by the output of PI::printBatches(true)
 */
std::vector<std::vector<std::vector<std::vector<int>>>> readBatches(const std::string& filename);

#endif  // INPUTS_HPP
