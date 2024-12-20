#include "problem_instance.hpp"
#include<tuple>

// Worker constructor
Worker::Worker(std::vector<double> params, double shift_time) {
  params_ = params;
  execution_time_ = 0;
}

// Worker destructor
Worker::~Worker() {
  clearWorker();
}

void Worker::clearWorker() {
  for (auto batch : batches_) {
    for (auto order : *batch) {
        delete order;
    }
    delete batch;
  }
  batches_.clear();
  // No need to delete elements of fatigue_levels_ since it's not a list of pointers
  fatigue_levels_.clear();
}

// cout
void Worker::printWorker() {
  std::cout << " ~ W" << params_[0] << ": ";
  int idx = 0;
  for (auto batch : batches_) {
    // Skip empty batches
    if (batch->empty()) continue;
    std::cout << "\tB" << idx << ": \t{";
    for (auto order : *batch) {
      std::cout << "{";
      for (auto prod : *order) {
        std::cout << prod << ",";
      }
      std::cout << "\b}; ";
    }
    std::cout << "\b\b}\n";
    idx++;
  }
  std::cout << "\n";
}

// PI constructor
PI::PI(std::string& namescheme,
       InputData data, std::vector<int>& workers, double shift_time,
       std::vector<char>& function_ids) {
  namescheme_ = namescheme;
  shift_time_ = shift_time;
  function_ids_ = function_ids;
  // Check if data and idx_workers are valid
  bool val = inputValidator(data, workers);
  if (!val) {
    std::cout << "! Invalid inputs.\n";
    // Stop program
    exit(1);
  }
  data_ = data;
  for (int i = 0; i < workers.size(); i++) {
    workers_.push_back(new Worker(data.worker_data[workers[i]], shift_time_));
  }
}

// PI destructor
PI::~PI() {
  clearPI();
}

void PI::clearPI() {
  // Delete workers
  for (auto worker : workers_) {
    // delete worker;
    worker->clearWorker();
    // std::cout << "Worker deleted.\n";
  }
  workers_.clear();
}

void PI::initializeBatches(int batch_capacity) {  // Still might cause to not add order in the case each worker surpasses shift time
  batch_capacity_ = batch_capacity;
  // Initialize all workers with one empty batch
  for (auto worker : workers_) {
    std::list<std::vector<int> *> *batch = new std::list<std::vector<int> *>;
    worker->appendBatch(batch);
  }
  for (auto order : data_.order_lines) {
    double best_time = std::numeric_limits<double>::max();
    Worker *best_worker = nullptr;
    std::list<std::vector<int> *> *best_batch = nullptr;
    for (auto worker : workers_) {
      for (auto batch : worker->getBatches()) {
        // Check if order can be added to batch
        if (sizeOfListOfVectors(*batch) + order.size() <= batch_capacity_) {
          // Add order to batch
          batch->push_back(new std::vector<int>(order));
          // Compute time to execute batch
          double time = newComputeFatiguePerformance();
          // Check if best solution
          if (time < best_time && checkFeasibility()) {
            best_time = time;
            best_worker = worker;
            best_batch = batch;
          }
          // Remove order from batch
          batch->pop_back();
        }
      }
    }
    if (best_worker == nullptr) {
      // Add empty batch to each worker
      for (auto worker : workers_) {
        std::list<std::vector<int> *> *batch = new std::list<std::vector<int> *>;
        worker->appendBatch(batch);
      }
      for (auto worker : workers_) {
        for (auto batch : worker->getBatches()) {
          // Check if order can be added to batch
          if (sizeOfListOfVectors(*batch) + order.size() <= batch_capacity_) {
            // Add order to batch
            batch->push_back(new std::vector<int>(order));
            // Compute time to execute batch
            double time = newComputeFatiguePerformance();
            // Check if best solution
            if (time < best_time && checkFeasibility(true)) {
              best_time = time;
              best_worker = worker;
              best_batch = batch;
            }
            // Remove order from batch
            batch->pop_back();
          }
        }
      }
    }
    // Add order to best batch
    best_batch->push_back(new std::vector<int>(order));
  }
  // add an empty batch to each worker
  for (auto worker : workers_) {
    std::list<std::vector<int> *> *batch = new std::list<std::vector<int> *>;
    worker->appendBatch(batch);
  }
  // initialize Fmax and related idx
  currFmax_ = -1;
  FmaxWorkerBatchId_ = std::make_pair(-1, -1);
  // initialize batches_time_fend_ and changed_batches_
  newComputeFatiguePerformance(true);
}

double PI::newComputeFatiguePerformance(bool recomputeAll) {
  double tot_time = 0;
  currFmax_ = -1;
  for (auto worker : workers_) {
    tot_time += newComputeFatiguePerformance(worker, recomputeAll);
  }
  return tot_time;
}

double PI::newComputeFatiguePerformance(Worker *worker, bool recomputeAll) {
  // Recompute only for specified worker  
  worker->clearTime();
  double time = 0;
  if (worker->batchesConfig() && !recomputeAll) {  // if changed_batches_ and batches_time_fend_ have been initialized
    int batch_id = 0;
    bool change_happened_before = false;
    double batch_time;
    double Fend = worker->getParams()[1];
    for (auto batch : worker->getBatches()) {
      // Reuse data if batch has not changed
      if (!worker->batchIsChanged(batch_id) && !change_happened_before) {
        if (!batch->empty()) {
          time += worker->getBatchesTimeFend(batch_id).first;
          Fend = worker->getBatchesTimeFend(batch_id).second;
          if (Fend > currFmax_) {
            currFmax_ = Fend;
            FmaxWorkerBatchId_ = std::make_pair(worker->getParams()[0], batch_id);
          }
        }
        batch_id++;
        continue;
      }
      // std::cout << "PI: Worker " << worker->getParams()[0] << " batch " << batch_id << " changed.\t";
      // Skip empty batches
      if (batch->empty()){
        worker->updateBatchTimeFend(batch_id, 0, Fend);
        worker->updateChangedBatch(batch_id, false);
        batch_id++;
        change_happened_before = true;
        continue;
      }
      
      // // print batch
      // std::cout << "Batch: ";
      // for (auto order : *batch) {
      //   std::cout << "{";
      //   for (auto prod : *order) {
      //     std::cout << prod << ",";
      //   }
      //   std::cout << "\b}; ";
      // }
      // std::cout << "\b\b\n";

      // Get route sequence
      std::vector<int> order_lines_flat;
      for (auto order : *batch) {
        for (auto prod : *order) {
          order_lines_flat.push_back(prod);
        }
      }

      std::tie(batch_time, Fend) = computeSshapePerformance(order_lines_flat, data_.loc_mat,
                                                            {data_.w_cross, data_.wy, static_cast<double>(data_.n_loc_per_aisle)},
                                                            data_.pen_mat, data_.time_mat,
                                                            function_ids_,
                                                            worker->getParams()[0],
                                                            Fend,
                                                            worker->getParams()[4],
                                                            worker->getParams()[2],
                                                            worker->getParams()[3],
                                                            worker->getParams()[5],
                                                            worker->getParams()[6]);
      time += batch_time;
      worker->updateBatchTimeFend(batch_id, batch_time, Fend);
      worker->updateChangedBatch(batch_id, false);
      if (Fend > currFmax_) {
        currFmax_ = Fend;
        FmaxWorkerBatchId_ = std::make_pair(worker->getParams()[0], batch_id);
      }
      batch_id++;
      change_happened_before = true;
    }
    // Update worker execution time
    worker->updateExecutionTime(time);
    return time;
  } else if (recomputeAll) {
    int batch_id = 0;
    double batch_time;
    double Fend = worker->getParams()[1];
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()){
        worker->appendBatchTimeFend(std::make_pair(0, Fend));
        worker->appendChangedBatch(false);
        worker->appendAvgFatigue(-1);  // do not consider empty batches when computing avg worker fatigue level
        batch_id++;
        continue;
      }
      // Get route sequence
      std::vector<int> order_lines_flat;
      for (auto order : *batch) {
        for (auto prod : *order) {
          order_lines_flat.push_back(prod);
        }
      }
      double Favg = 0;
      std::tie(batch_time, Fend) = computeSshapePerformance(order_lines_flat, data_.loc_mat,
                                                            {data_.w_cross, data_.wy, static_cast<double>(data_.n_loc_per_aisle)},
                                                            data_.pen_mat, data_.time_mat,
                                                            function_ids_,
                                                            worker->getParams()[0],
                                                            Fend,
                                                            worker->getParams()[4],
                                                            worker->getParams()[2],
                                                            worker->getParams()[3],
                                                            worker->getParams()[5],
                                                            worker->getParams()[6],
                                                            &Favg);
      worker->appendBatchTimeFend(std::make_pair(batch_time, Fend));
      worker->appendChangedBatch(false);
      worker->appendAvgFatigue(Favg);

      if (Fend > currFmax_) {
        currFmax_ = Fend;
        FmaxWorkerBatchId_ = std::make_pair(worker->getParams()[0], batch_id);
      }
      batch_id++;
      time += batch_time;
    }
    // Update worker execution time
    worker->updateExecutionTime(time);
    return time;
  } else {
    double batch_time;
    double Fend = worker->getParams()[1];
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) continue;
      // Get route sequence
      std::vector<int> order_lines_flat;
      for (auto order : *batch) {
        for (auto prod : *order) {
          order_lines_flat.push_back(prod);
        }
      }
      std::tie(batch_time, Fend) = computeSshapePerformance(order_lines_flat, data_.loc_mat,
                                                            {data_.w_cross, data_.wy, static_cast<double>(data_.n_loc_per_aisle)},
                                                            data_.pen_mat, data_.time_mat,
                                                            function_ids_,
                                                            worker->getParams()[0],
                                                            Fend,
                                                            worker->getParams()[4],
                                                            worker->getParams()[2],
                                                            worker->getParams()[3],
                                                            worker->getParams()[5],
                                                            worker->getParams()[6]);
      time += batch_time;
    }
    // Update worker execution time
    worker->updateExecutionTime(time);
    return time;
  }
}

// Total performance as sum of worker execution times
double PI::getTotalPerformance() {
  double performance = 0;
  for (auto worker : workers_) {
    performance += worker->getExecutionTime();
  }
  return performance;
}


// ---- only on final solution for detailed analysis ----
void PI::computeRoutes() {
  /* Requires batches to be already assigned */
  // Sum route times for each batch of each worker
  double time;
  for (auto worker : workers_) {
    // Delete existing routes
    worker->clearRoutes();
    time = 0;  // Time if fatigue was not present
    // Compute routes for each batch
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) continue;
      // Get route sequence
      std::vector<int> order_lines_flat;
      for (auto order : *batch) {
        for (auto prod : *order) {
          order_lines_flat.push_back(prod);
        }
      }
      // New route
      std::list<std::vector<double>> route;
      // Compute route and append to route
      std::list<std::vector<double>> route_seq_and_time = sShapeWithTimeVector(order_lines_flat, data_.loc_mat,
                                                                {data_.w_cross, data_.wy, static_cast<double>(data_.n_loc_per_aisle)});

      for (auto node : route_seq_and_time) {
        route.push_back(node);
        // Second element of each node is time to node
        time += node[1];
      }
      // Append route to worker
      worker->appendRoute(route);
    }
    // worker->updateExecutionTime(time);
  }
}

void PI::computeFatigueLevelAndEffect() {
  /* Requires routes to be already computed */
  // Compute fatigue level for each route
  for (auto worker : workers_) {
    // Delete existing fatigue levels
    worker->clearFatigueLevels();
    // Compute time to execute all batches for each worker
    double time = 0;

    // tmp for check
    double batch_time;
    int idx = 0;
    // std::cout << "Worker " << worker->getParams()[0] << ":\n";

    double Fstart = worker->getParams()[1];
    for (auto route_seq_and_time : worker->getRoutes()) {
      
      // tmp for checkc
      batch_time = 0;
      // std::cout << "\tBatch id: " << idx << "\n";

      // Get FT evolution
      std::pair<double, std::list<double>> ft_evolution;
      ft_evolution = computeFTevolution(route_seq_and_time, function_ids_,
                                        data_.pen_mat, data_.time_mat,
                                        worker->getParams()[0],
                                        Fstart,
                                        worker->getParams()[4],
                                        worker->getParams()[2],
                                        worker->getParams()[3],
                                        worker->getParams()[5],
                                        worker->getParams()[6]);
      Fstart = ft_evolution.second.back();
      // Save fatigue level
      std::list<double> fatigue_level;
      fatigue_level.assign(ft_evolution.second.begin(), ft_evolution.second.end());

      worker->appendFatigueLevel(fatigue_level);
      // Update worker execution time
      time += ft_evolution.first;
      
      // tmp - Print last fatigue level and total batch time
      batch_time += ft_evolution.first;
      idx++;
      // std::cout << "\t\tFend: " << fatigue_level.back() << "\n";
      // std::cout << "\t\tBatch time: " << batch_time << "(" << time << ")\n";

    }
    // Total time for a worker to execute all its batches
    worker->updateExecutionTime(time);
  }
}


// cout
void PI::printBatches(bool print_empty) {
  std::cout << " > BATCHES:\n";
  int idx;
  for (auto worker : workers_) {
    idx = 0;
    std::cout << "W" << worker->getParams()[0] << ": ";
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) {
        if (print_empty) {
          std::cout << "\tBe: \t{";
          std::cout << "}\n";
        }
        continue;
      }
      std::cout << "\tB" << idx << ": \t{";
      for (auto order : *batch) {
        std::cout << "{";
        for (auto prod : *order) {
          std::cout << prod << ",";
        }
        std::cout << "\b}; ";
      }
      std::cout << "\b\b}\n";
      idx++;
    }
    std::cout << "\n";
  }
}
void PI::saveBatches(bool print_empty, std::string& outname, std::string& folder_name) {
  std::string filename = "../data/results/" + folder_name + "/" + namescheme_ + outname + ".txt";
  std::ofstream file;
  file.open(filename);
  std::cout << "Writing to ../data/results/" << folder_name << "/" << namescheme_ << outname << ".txt ... ";
  int idx;
  for (auto worker : workers_) {
    idx = 0;
    file << "W" << worker->getParams()[0] << ": ";
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) {
        if (print_empty) {
          file << "\tBe: \t{";
          file << "}\n";
        }
        continue;
      }
      file << "\tB" << idx << ": \t{";
      for (auto order : *batch) {
        file << "{";
        for (auto prod : *order) {
          file << prod << ",";
        }
        file.seekp(-1, std::ios_base::end);
        file << "}; ";
      }
      file.seekp(-2, std::ios_base::end);
      file << "}\n";
      idx++;
    }
    file << "\n";
  }
  file.close();
  std::cout << "Done.\n";
}

int sizeOfListOfVectors(const std::list<std::vector<int> *>& list_of_vectors) {
  int size = 0;
  for (auto vector : list_of_vectors) {
    size += vector->size();
  }
  return size;
}

double PI::computeTotalPerformance() {
  // Reset execution time
  for (auto worker : workers_) {
    worker->clearTime();
  }
  // Compute routes
  computeRoutes();
  // Compute fatigue level
  computeFatigueLevelAndEffect();
  // Return total performance
  return getTotalPerformance();
}

bool PI::checkFeasibility(bool tr) {
  // Check compliance with shift time for every worker
  for (auto worker : workers_) {
    if (worker->getExecutionTime() > shift_time_) {
      // std::cout << "FeasibilityWarning:\t";
      // std::cout << "Worker " << worker->getParams()[0] << " exceeds shift time.\n";
      // raise error
      if (tr) {
        std::exception ec;
        throw ec;
      }
      return false;
    }
    // Check batches capacity
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) continue;
      if (sizeOfListOfVectors(*batch) > batch_capacity_) {
        // std::cout << "FeasibilityWarning:\t";
        // std::cout << "Worker " << worker->getParams()[0] << " exceeds batch capacity.\n";
        return false;
      }
    }
  }

  return true;
}

void PI::saveRecapToFile(std::string& outname, std::string& folder_name) {
  std::string filename = "../data/results/" + folder_name + "/" + namescheme_ + outname + ".txt";
  std::ofstream file;
  file.open(filename);
  std::cout << "Writing to ../data/results/" << folder_name << "/" << namescheme_ << outname << ".txt ... ";
  int idx;
  std::vector<int> order_lines_flat;
  std::vector<std::vector<double>> times;

  // General pi data
  file << "Shift time: " << shift_time_ / 3600 << "h\n";
  file << "Batch capacity: " << batch_capacity_ << "\n";
  file << "Workers: ";
  for (auto worker : workers_) {
    file << worker->getParams()[0] << ", ";
  }
  file.seekp(-2, std::ios_base::end);
  file << "\n";
  file << "Fatigue thresholds: ";
  for (auto worker : workers_) {
    file << worker->getParams()[5] << ", ";
  }
  file.seekp(-2, std::ios_base::end);
  file << "\n";
  file << "Function ids: ";
  for (auto id : function_ids_) {
    file << id << ", ";
  }
  file.seekp(-2, std::ios_base::end);
  file << "\n\n";

  // Worker data
  file << "Total performance: " << newComputeFatiguePerformance(true) << "s\n";
  for (auto worker : workers_) {
    // Tot performance
    file << "W" << worker->getParams()[0] << ": ";
    file << "Tot time = " << newComputeFatiguePerformance(worker,true) << "s\n";
    // Batches
    idx = 0;
    for (auto batch : worker->getBatches()) {
      // Skip empty batches
      if (batch->empty()) continue;
      file << "\tB" << idx << ": \t{";
      for (auto order : *batch) {
        file << "{";
        for (auto prod : *order) {
          file << prod << ",";
        }
        file.seekp(-1, std::ios_base::end);
        file << "}, ";
      }
      file.seekp(-2, std::ios_base::end);
      file << "}\n";
      idx++;
    }
    // Routes
    idx = 0;
    for (auto route : worker->getRoutes()) {
      // Skip empty routes
      if (route.empty()) continue;
      file << "\tR" << idx << ": \t[";
      for (auto node : route) {
        file << node[0] << ", ";
      }
      file.seekp(-2, std::ios_base::end);
      file << "]\n";
      idx++;
    }
    idx = 0;
    for (auto fatigue : worker->getFatigueLevels()) {
      file << "\tF" << idx << ": ";
      file << "\t(";
      for (auto level : fatigue) {
        file << level << ", ";
      }
      file.seekp(-2, std::ios_base::end);
      file << ")\n";
      idx++;
    }
    file << "\n";
    // Get times per task
    idx = 0;
    double Fstart = worker->getParams()[1];
    for (auto route_seq_and_time : worker->getRoutes()) {
      // Get FT evolution
      std::pair<double, std::list<double>> ft_evolution;
      ft_evolution = computeFTevolution(route_seq_and_time, function_ids_,
                                        data_.pen_mat, data_.time_mat,
                                        worker->getParams()[0],
                                        Fstart,
                                        worker->getParams()[4],
                                        worker->getParams()[2],
                                        worker->getParams()[3],
                                        worker->getParams()[5],
                                        worker->getParams()[6]);
      times = computeFTtimesPerTask(route_seq_and_time, function_ids_,
                                    data_.pen_mat, data_.time_mat,
                                    worker->getParams()[0],
                                    Fstart,
                                    worker->getParams()[4],
                                    worker->getParams()[2],
                                    worker->getParams()[3],
                                    worker->getParams()[5],
                                    worker->getParams()[6]);
      Fstart = ft_evolution.second.back();
      file << "\tT" << idx << ": ";
      file << "\t[";
      for (auto time : times) {
        file << "{" << time[0] << ", " << time[1] << ", " << time[2] << ", " << time[3] << "}, ";
      }
      file.seekp(-2, std::ios_base::end);
      file << "]\n";
      idx++;
    }
    file << "\n";
  }

  file.close();
  std::cout << "Done.\n";
}

void PI::updateWorkers(std::vector<Worker> workers) {
  // Delete all workers and replace them with the new ones
    for (auto worker : workers_) {
      worker->clearWorker();
    }
    workers_.clear();
    for (auto worker : workers) {
      Worker *wnew = new Worker(worker);
      workers_.push_back(wnew);
    }
    newComputeFatiguePerformance(true);
}

void PI::rebuildBatchesFromFile(std::string& filepath) {
  // Use function readBatches to update all workers
  std::vector<std::vector<std::vector<std::vector<int>>>> batches;
  batches = readBatches(filepath);
  // Empty worker batches
  for (auto worker : workers_) {
    worker->clearWorker();
  }
  // Update workers
  int idx = 0;
  for (auto worker : workers_) {
    for (auto batch : batches[idx]) {
      std::list<std::vector<int> *> *newbatch = new std::list<std::vector<int> *>;
          // std::cout << batch.size() << std::endl;
      for (auto order : batch) {
        if (order.size() > 0){
          std::vector<int> orderList;
          for (auto prod : order) {
            orderList.push_back(prod);
          }
          newbatch->push_back(new std::vector<int>(orderList));
        } 
      }
      worker->appendBatch(newbatch);
    }
    idx++;
  }
  // initialize Fmax and related idx
  currFmax_ = -1;
  FmaxWorkerBatchId_ = std::make_pair(-1, -1);
  // initialize batches_time_fend_ and changed_batches_
  newComputeFatiguePerformance(true);
}
