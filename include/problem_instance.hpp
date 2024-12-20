/**
* @file
* @version 1.0
*
* @section DESCRIPTION
* Definition of classes that define a problem instance.
* The main characterization of a problem instance is the Worker instances and their batches.
*/
#ifndef PROBLEM_INSTANCE_HPP
#define PROBLEM_INSTANCE_HPP

#include <string>
#include <list>
#include <vector>
#include <iostream>
#include "inputs.hpp"
#include "routing.hpp"
#include "fatigue.hpp"


/**
* @brief Worker class for each picker.
* @details Each worker has specific parameters and the solution is described by their batches.
*          Worker paramters are: Fstart,recovery_rate,rest_rate,time_correction_factor,Fth,(Flow)
*/
class Worker {
 public:
  explicit Worker(std::vector<double> params, double shift_time);
  ~Worker();
  void clearWorker();

  /**
   * @brief Appends a batch to the list of batches
   * @param batch List of orders to append as new batch
  */
  void appendBatch(std::list<std::vector<int> *> *batch) { batches_.push_back(batch); }
  /**
   * @brief Returns the list of batches
  */
  std::list<std::list<std::vector<int> *> *> getBatches() { return batches_; }
  /**
   * @brief Insert a new batch before index
  */
  void insertBatch(int index, std::list<std::vector<int> *> batch) {
    if (index >= batches_.size()) {
      // Append batch at the end
      appendBatch(new std::list<std::vector<int> *>(batch));
    } else {
      auto it = batches_.begin();
      std::advance(it, index);
      batches_.insert(it, new std::list<std::vector<int> *>(batch));
    }
    // Add 1 element to changed_batches_; batces_time_fend_; batches_avg_fatigue_
    changed_batches_.push_back(true);
    batches_time_fend_.push_back(std::make_pair(0, 0));
    batches_avg_fatigue_.push_back(0);
  }
  /**
   * @brief Remove a batch at index
  */
  void removeBatch(int index) {
    auto it = batches_.begin();
    std::advance(it, index);
    batches_.erase(it);
    // Remove 1 element from changed_batches_; batces_time_fend_; batches_avg_fatigue_
    changed_batches_.pop_back();
    batches_time_fend_.pop_back();
    batches_avg_fatigue_.pop_back();
    // If worker is empty, append an empty batch
    if (batches_.size() < 2) {
      appendBatch(new std::list<std::vector<int> *>);
      changed_batches_.push_back(true);
      batches_time_fend_.push_back(std::make_pair(0, 0));
      batches_avg_fatigue_.push_back(0);
    }
  }
  int getBatchId(std::list<std::vector<int> *> *batch) {
    int batch_id = 0;
    for (auto it = batches_.begin(); it != batches_.end(); it++) {
      if (*it == batch) {
        return batch_id;
      }
      batch_id++;
    }
    return -1;
  }
  /**
   * @brief Returns the worker-specific paramters. 
   * Fstart,recovery_rate,rest_rate,time_correction_factor,Fth,(Flow)
  */
  std::vector<double> getParams() { return params_; }
  /**
   * @brief Updates the execution time of the worker by time
   * @param time Time to add to the execution time
  */
  void updateExecutionTime(double time) { execution_time_ += time; }
  /**
   * @brief Returns the current execution time of the worker
  */
  double getExecutionTime() { return execution_time_; }
  /**
   * @brief Appends a route to the list of routes 
   * @details This is used only to save full details of worker
   * @param route Route to append
  */
  void appendRoute(std::list<std::vector<double>> route) { routes_.push_back(route); }
  /**
   * @brief Returns the list of routes
  */
  std::list<std::list<std::vector<double>>> getRoutes() { return routes_; }
  /**
   * @brief Appends a fatigue level to the list of fatigue levels
   * @details This is used only to save full details of worker
   * @param fatigue_level Fatigue level to append
  */
  void appendFatigueLevel(std::list<double>fatigue_level) { fatigue_levels_.push_back(fatigue_level); }
  /**
   * @brief Returns the list of fatigue levels
  */
  std::list<std::list<double>> getFatigueLevels() { return fatigue_levels_; }
  void clearRoutes() { routes_.clear(); }
  void clearFatigueLevels() { fatigue_levels_.clear(); }
  void clearTime() { execution_time_ = 0; }

  // eval efficiency
  bool batchIsEmpty(int batch_id) {
    auto it = batches_.begin();
    std::advance(it, batch_id);
    return (*it)->empty();
  }
  bool batchesConfig() {return !batches_time_fend_.empty(); }
  void appendBatchTimeFend(std::pair<double, double> batch_time_fend) { batches_time_fend_.push_back(batch_time_fend); }
  std::pair<double, double> getBatchesTimeFend( int batch_id) { return batches_time_fend_[batch_id]; }
  void updateBatchTimeFend(int batch_id, double batch_time, double batch_fend) {
    auto it = batches_time_fend_.begin();
    std::advance(it, batch_id);
    it->first = batch_time;
    it->second = batch_fend;
  }
  void appendChangedBatch(bool changed) { changed_batches_.push_back(changed); }
  bool batchIsChanged(int batch_id) { return changed_batches_[batch_id]; }
  void updateChangedBatch(int batch_id, bool changed) {
    auto it = changed_batches_.begin();
    std::advance(it, batch_id);
    *it = changed;
  }
  void appendAvgFatigue(double avg_fatigue) { batches_avg_fatigue_.push_back(avg_fatigue); }
  double getAvgFatigue(int batch_id) { return batches_avg_fatigue_[batch_id]; }
  double getAvgFatigue() {
    int batch_id = 0;
    double avg_fatigue = 0;
    for (auto it = batches_avg_fatigue_.begin(); it != batches_avg_fatigue_.end(); it++) {
      if (*it > 0) avg_fatigue += *it;
      if (*it > 0) batch_id++;
    }
    return avg_fatigue / batch_id;
  }

  // cout
  /**
   * @brief Print worker id and batches
  */
  void printWorker();

  // Deep copy constructor
  Worker(const Worker& other)
    : params_(other.params_), execution_time_(other.execution_time_) {
        
    for (auto batch : other.batches_) {
        std::list<std::vector<int> *> *newbatch = new std::list<std::vector<int> *>;
        for (auto order : *batch) {
          std::vector<int> orderList;
          for (auto prod : *order) {
            orderList.push_back(prod);
          }
        newbatch->push_back(new std::vector<int>(orderList));        
      }      
      batches_.push_back(new std::list<std::vector<int> *>(*newbatch));
    }
    // for (auto fatigue_level : other.fatigue_levels_) {
    //   fatigue_levels_.push_back(fatigue_level);
    // }
  }

 private:
  // Worker parameters
  std::vector<double> params_;
  // Route for each batch: pairs of {loc, time}
  std::list<std::list<std::vector<double>>> routes_;
  // List of batches, each batch is a list of orders
  std::list<std::list<std::vector<int> *> *> batches_;
  // Bool list for changed batches
  std::vector<bool> changed_batches_;
  // Pair {batch_time, Fend} in a list for each batch
  std::vector<std::pair<double, double>> batches_time_fend_;
  // Average batch fatigue level in a list for each batch
  std::vector<double> batches_avg_fatigue_;
  // Working time for executing all assigned task
  double execution_time_;
  // Fatigue level at each task in each batch
  std::list<std::list<double>> fatigue_levels_;
};

/**
* @brief Problem instance class
* @details The main characterization of a problem instance is the Worker instances and their batches.
*/
class PI {
 public:
  PI(std::string& namescheme, InputData data, std::vector<int>& workers, double shift_time, std::vector<char>& function_ids);
  ~PI();
  void clearPI();

  /**
   * @brief Initialize batches with greedy sequential insertion
   * @details Leaves empty batches for each worker
  */
  void initializeBatches(int batch_capacity);
  /**
   * @brief Compute and stores routes for each worker
  */
  void computeRoutes();
  /**
   * @brief Compute and stores fatigue levels for each worker
  */
  void computeFatigueLevelAndEffect();
  /**
   * @brief Compute the total performance of the solution by computing routes and fatigue levels
   *        usign computeRoutes() and computeFatigueLevelAndEffect() 
   * @details Old apprach. Now updated with newComputeFatiguePerformance()
  */
  double computeTotalPerformance();
  /**
   * @brief Check if the solution is feasible
   * @details Checks compliance with batch capacity and total shift time
  */
  bool checkFeasibility(bool tr = false);
  std::list<Worker *> getWorkers() { return workers_; }
  int getMaxCapacity() { return batch_capacity_; }
  /**
   * @brief Returns the total performance of the solution (already computed)
  */
  double getTotalPerformance();
  /**
   * @brief Substitutes the current workers with the ones in input
  */
  void updateWorkers(std::vector<Worker> workers);
  std::string getOutNamescheme() { return namescheme_; }
  /**
   * @brief Compute the total performance of the current solution
   * @details New approach: computes the total performance using the fatigue levels and routes as joined without saving data
   * @param worker Worker to compute the performance for. If unspecified, compute for all
  */
  double newComputeFatiguePerformance(bool recomputeAll = false);
  double newComputeFatiguePerformance(Worker *worker, bool recomputeAll = false);
  // cout
  /**
   * @brief Print batches of all workers to cout
   * @param print_empty If true, print empty batches too
  */
  void printBatches(bool print_empty = false);
  /**
   * @brief Print complete solution recap to file, including routes and fatigue levels
   * @param outname File name to print to
   * @param folder_name Folder name to save the file
  */
  void saveRecapToFile(std::string& outname, std::string& folder_name);
  /**
   * @brief Print only batches to file. Same format as printBatches()
   * @param print_empty If true, print empty batches too
   * @param outname File name to print to
   * @param folder_name Folder name to save the file
  */
  void saveBatches(bool print_empty, std::string& outname, std::string& folder_name);
  /**
   * @brief Updates current solution batches from file
   * @details File format is the one generated from ./data/instances/batches_from_file.ipynb
   * @param filepath File name to read from
  */
  void rebuildBatchesFromFile(std::string& filepath);

  // eval efficiency
  void updateChangedBatchForWorker(Worker * worker, int batch_id, bool changed) {
    worker->updateChangedBatch(batch_id, changed);
  }

  // Track of workers/batches for operators
  std::pair<int, int> getFmaxWorkerBatchId() { return FmaxWorkerBatchId_; }
  // std::pair<int, int> getMtrigWorkerBatchIds();

  // Get avg penibility of an order assigned to a worker
  double getAvgPenibility(std::vector<int> *order, Worker *worker) {
    double avg_pen = 0;
    for (auto prod : *order) {
      avg_pen += data_.pen_mat[prod][worker->getParams()[0]];
    }
    return avg_pen / order->size();
  }

 private:
  std::string namescheme_;
  int batch_capacity_;
  double shift_time_;
  InputData data_;
  std::list<Worker *> workers_;
  std::vector<char> function_ids_;
  double currFmax_;
  std::pair<int, int> FmaxWorkerBatchId_;
};

/**
* @brief Size of a list of vectors
*/
int sizeOfListOfVectors(const std::list<std::vector<int> *>& list_of_vectors);


#endif // PROBLEM_INSTANCE_HPP
