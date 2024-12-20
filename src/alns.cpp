#include "alns.hpp"

ALNS::ALNS(PI * ptr, int nr_operators, std::vector<int> idx_operators) {
  PIptr_ = ptr;
  idx_operators_ = idx_operators;
  op_ = Operators(ptr);
  // // Compute initial solution performance
  // PIptr_->newComputeFatiguePerformance();
  // Initialize best_sol_ as a deep copy of current PI Workers
  for (auto worker : ptr->getWorkers()) {
    Worker wnew(*worker);
    best_sol_.push_back(wnew);
  }
  best_perf_ = ptr->newComputeFatiguePerformance();
  // Same for curr_sol_
  for (auto worker : ptr->getWorkers()) {
    Worker wnew(*worker);
    curr_sol_.push_back(wnew);
  }
  curr_perf_ = ptr->newComputeFatiguePerformance();
  // Initialize new_perf_
  new_perf_ = curr_perf_;
  // Params
  weights_ = std::vector<double>(nr_operators, 0.0);
  for (int i = 0; i < idx_operators.size(); i++) {
    weights_[idx_operators[i]] = 0.5;
  }
  usage_nrs_ = std::vector<int>(nr_operators, 0);
  scores_ = std::vector<double>(nr_operators, 0.0);
  // Track total performance
  tot_run_times_ = std::vector<double>(nr_operators, 0.0);
  tot_usage_nrs_ = std::vector<double>(nr_operators, 0.0);
  tot_improvement_ = std::vector<double>(nr_operators, 0.0);
}

ALNS::~ALNS(void) {
  // Clear best_sol_ and curr_sol_
  for (auto worker : best_sol_) {
    worker.clearWorker();
  }
  best_sol_.clear();
  for (auto worker : curr_sol_) {
    worker.clearWorker();
  }
  curr_sol_.clear();
}

int ALNS::randomOperator() {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::vector<double> probabilities;
  // Normalize weights vector
  double sum = 0;
  for (auto w : weights_) {
    sum += w;
  }
  for (auto w : weights_) {
    probabilities.push_back(w / sum);
  }
  // Create a discrete distribution based on the probabilities
  std::discrete_distribution<> distribution(probabilities.begin(), probabilities.end());

  // // tmp - print all prob
  // for (auto p : probabilities) {
  //   std::cout << p << "\n";
  // }
  // std::cout << "\n\n";

  // Generate a random index
  int randomIndex = distribution(gen);
  return randomIndex;
}

void ALNS::printSolutions() {
  std::cout << "Best solution: " << best_perf_ << "\n";
  for (auto worker : best_sol_) {
    worker.printWorker();
  }
  std::cout << "Current solution: " << curr_perf_ << "\n";
  for (auto worker : curr_sol_) {
    worker.printWorker();
  }
  std::cout << "New solution: " << PIptr_->getTotalPerformance() << "\n";
  for (auto worker : PIptr_->getWorkers()) {
    worker->printWorker();
  }
}

void ALNS::executeStep(int idx_op, int Q) {
  // Random number between 1 and q
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, Q);
  int q = dis(gen);
  
  // // DEBUGGING
  // std::cout << "OP " << idx_op << "\n";
  // PIptr_->printBatches(true);

  op_.execute(idx_op, q);
  new_perf_ = PIptr_->newComputeFatiguePerformance();
  
  // std::cout << "\tstep executed. Feasibility " << PIptr_->checkFeasibility() << "\n";

}

void ALNS::run(int n_segments, int iter_per_seg, std::vector<double> sigmas,
               double r, double cool_rate, double worsening_factor,
               int Qmax, int reset_at, int overwrite_for, double shakeT, double shakec,
               bool track_segements, bool track_iter, std::string& folder_name) {
  double temp = worsening_factor * curr_perf_ / log(1 / 0.5);
  std::ofstream file;
  std::ofstream file2;
  if (track_segements) {
    // Write to file
    file.open("../data/results/" + folder_name + "/" + PIptr_->getOutNamescheme() + "segments.txt");
  }
  if (track_iter) {
    // Write to file
    file2.open("../data/results/" + folder_name + "/" + PIptr_->getOutNamescheme() + "iterations.txt");
    file2 << "Curr\tBest\tTemp\tOp\tWeights (in)\n";
  }
  // Run ALNS
  int n_not_improved = 0;  // track nr of iterations without improvement in best sol
  int counter = 0;  // track nr of iterations with acceptance of worse solution
  bool toggle_SA = true;  // If false overwrite SA
  for (int i = 0; i < n_segments; i++) {
    if (track_segements) {
      file << "# Segment "  << i << " " << "\n";
      file << "\tTemperature (start): " << temp << "\n";
      file << "\tWeights (start): ";
      for (auto w : weights_) {
        file << w << " ";
      }
      file << std::endl;
    }
    for (int j = 0; j < iter_per_seg; j++) {
      int idx_op = randomOperator();
      n_not_improved++;
      // Track run time
      auto start = std::chrono::high_resolution_clock::now();
      executeStep(idx_op, Qmax);
      auto end = std::chrono::high_resolution_clock::now();

      // Update operator params
      usage_nrs_[idx_op]++;
      // Update operator total performance
      tot_run_times_[idx_op] += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      tot_usage_nrs_[idx_op]++;
      // Solutions
      if (new_perf_ < curr_perf_) {
        // Accept new solution
        curr_perf_ = new_perf_;
        curr_sol_.clear();
        for (auto worker : PIptr_->getWorkers()) {
          Worker wnew(*worker);
          curr_sol_.push_back(wnew);
        }
        // Update best solution
        if (curr_perf_ < best_perf_) {
          scores_[idx_op] += sigmas[0];
          tot_improvement_[idx_op] += best_perf_ - curr_perf_;
          best_perf_ = curr_perf_;
          n_not_improved = 0;
          // Update best_sol_
          best_sol_.clear();
          for (auto worker : PIptr_->getWorkers()) {
            Worker wnew(*worker);
            best_sol_.push_back(wnew);
          }
        } else {
            scores_[idx_op] += sigmas[1];
        }
      } else if (toggle_SA) {
        double rnum = (double) rand() / RAND_MAX;
        if (rnum < exp((curr_perf_ - new_perf_) / temp)) { // SA accept
          // std::cout << "New solution accepted\n";
          scores_[idx_op] += sigmas[2];
          // Accept new solution
          curr_perf_ = new_perf_;
          curr_sol_.clear();
          for (auto worker : PIptr_->getWorkers()) {
            Worker wnew(*worker);
            curr_sol_.push_back(wnew);
          }
        } else { // SA reject ...
          // std::cout << "New solution rejected\n";
          // Restore current solution
          PIptr_->updateWorkers(curr_sol_);
        }
      } else {
        // Accept any solution for Y iterations
        curr_perf_ = new_perf_;
        curr_sol_.clear();
        for (auto worker : PIptr_->getWorkers()) {
          Worker wnew(*worker);
          curr_sol_.push_back(wnew);
        }
        counter++;
        // Stop and revert to best
        if (counter >= overwrite_for) {
          toggle_SA = true;
          n_not_improved = 0;
          counter = 0;
          // // New current is the best solution
          // curr_perf_ = best_perf_;
          // curr_sol_.clear();
          // for (auto worker : best_sol_) {
          //   Worker wnew(worker);
          //   curr_sol_.push_back(wnew);
          // }
        }
      }
      // Update temperature
      temp = temp * cool_rate;
      // Toggle off SA for Y iterations if no improvement
      if (n_not_improved >= reset_at) {
        // Shake with empty_greedy
        toggle_SA = false;
        // std::cout << "shake at It " << i * iter_per_seg + j << std::endl;
        PIptr_->updateWorkers(best_sol_);
        // executeStep(21, Qmax);
        // Accept new solution
        curr_perf_ = new_perf_;
        curr_sol_.clear();
        for (auto worker : PIptr_->getWorkers()) {
          Worker wnew(*worker);
          curr_sol_.push_back(wnew);
        }
        // Set new SA params
        temp += worsening_factor * shakeT * curr_perf_ / log(1 / 0.5);
        cool_rate = shakec;
        
        n_not_improved = 0;
      }
      if (track_iter) {
        file2 << curr_perf_ << "\t" << best_perf_ << "\t" << temp << "\t" << idx_op << "\t";
        for (auto w : weights_) {
          file2 << w << ", ";
        }
        file2 << "\n";
      }
    }
    // Update weights
    updateWeights(r);
    if (track_segements) {
      file << "\tScores: ";
      for (auto s : scores_) {
        file << s << " ";
      }
      file << std::endl;
      file << "\tUsage nr: ";
      for (auto u : usage_nrs_) {
        file << u << " ";
      }
      file << std::endl;
      // Time until curr segment
      file << "Total runtime [min]: " << accumulate(tot_run_times_.begin(), tot_run_times_.end(), 0.0) / 60000.0 << std::endl;
      // PErformance
      file << "\tPerformances (after):" << "\n";
      file << "\t\tBest\t" << best_perf_ << "\n";
      file << "\t\tCurrent\t" << curr_perf_ << "\n";
      file << "\t\tNew\t" << PIptr_->getTotalPerformance() << "\n";
    }
    // Reset scores and usage_nrs_
    for (int k = 0; k < weights_.size(); k++) {
      scores_[k] = 0;
      usage_nrs_[k] = 0;
    }
  }
  if (track_segements) {file.close();}
  if (track_iter) {file2.close();}
}

void ALNS::updateWeights(double r) {
  // w[i] = w[i]*(1-r) + r * (s[i] / u[i])
  // // Print
  // std::cout << "Weights: ";
  // for (auto w : weights_) {
  //   std::cout << w << " ";
  // }
  for (int i = 0; i < weights_.size(); i++) {
    // std::cout << "Operator " << i << ":\t" << weights_[i] << "\n";
    // std::cout << "\tScore: " << scores_[i] << "\n";
    // std::cout << "\tUsage nr: " << usage_nrs_[i] << "\n";
    if (usage_nrs_[i] == 0) {
      // do not update
      continue;
    } else {
      weights_[i] = weights_[i] * (1 - r) + r * (scores_[i] / usage_nrs_[i]);
    }
  }
}

void ALNS::saveOperatorTotParams(std::string& folder_name) {
  // Write to file
  std::ofstream file;
  file.open("../data/results/" + folder_name + "/" + PIptr_->getOutNamescheme() + "operators.txt");
  // Recap params
  file << "Total runtime [ms]: " << accumulate(tot_run_times_.begin(), tot_run_times_.end(), 0.0) << "\n";
  file << "Total usages: " << accumulate(tot_usage_nrs_.begin(), tot_usage_nrs_.end(), 0.0) << "\n";
  file << "Total improvement (best): " << accumulate(tot_improvement_.begin(), tot_improvement_.end(), 0.0) << "\n";
  // Operator-specific
  file << "# Operator parameters" << "\n";
  file << "Op\tRuntime [ms]\tUsages\tImprovement\n";
  for (int i = 0; i < idx_operators_.size(); i++) {
    file << idx_operators_[i] << "\t" << tot_run_times_[idx_operators_[i]] << "\t\t" << tot_usage_nrs_[idx_operators_[i]] << "\t" << tot_improvement_[idx_operators_[i]] << "\n";
  }
  file.close();
}


void ALNS::bruteforce() {
  // take all PI orders and create all possible solutions, assume only one worker
  // orders are pointer is Worker batches list
  std::vector<std::vector<int> *> orders_all;
  for (auto worker : PIptr_->getWorkers()) {
    for (auto batch : worker->getBatches()) {
      for (auto order : *batch) {
        orders_all.push_back(order);
      }
    }
  }
  // Sort orders increasing by address
  std::sort(orders_all.begin(), orders_all.end(), [](std::vector<int> * a, std::vector<int> * b) {
    return a->front() < b->front();
  });
  // Print all orders and their addresses
  for (auto order : orders_all) {
    std::cout << order << " ";
    for (auto prod : *order) {
      std::cout << prod << " ";
    }
    std::cout << "\n";
  }
  // Create all possible solutions by permuting the orders
  std::vector<std::vector<int> *> orders_best;
  double best_perf = std::numeric_limits<double>::max();
  Worker * worker = PIptr_->getWorkers().front();
  int i = 0;
  do {
    // Create a new solution
    std::vector<std::vector<int> *> orders_new;
    for (auto order : orders_all) {
      std::vector<int> order_new;
      for (auto prod : *order) {
        order_new.push_back(prod);
      }
      orders_new.push_back(new std::vector<int>(order_new));
    }
    // From current permutation, form batches sequentially (abiding to capacity)
    std::list<std::list<std::vector<int> *> *> batches_new;
    int curr_capacity;
    for (auto order : orders_new) {
      // Check if order fits in current batch
      if (batches_new.empty() || curr_capacity + order->size() > PIptr_->getMaxCapacity()) {
        // Create new batch
        curr_capacity = order->size();
        std::list<std::vector<int> *> * batch_new = new std::list<std::vector<int> *>();
        batch_new->push_back(order);
        batches_new.push_back(batch_new);
      } else {
        // Add to last batch
        batches_new.back()->push_back(order);
        curr_capacity += order->size();
      }
    }
    // Update PI with new solution
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (auto batch : batches) {
      batch->clear();
    }
    // Remove all batches
    worker->clearWorker();
    // Add new batches
    for (auto batch : batches_new) {
      worker->appendBatch(batch);
    }
    // Compute performance
    double perf = PIptr_->newComputeFatiguePerformance(true);
    if (perf < best_perf) {
      best_perf = perf;
      orders_best = orders_new;
    }
    i++;
    if (i % 362880 == 0) {
      std::cout << i/362880 << std::endl;
    }
  } while (std::next_permutation(orders_all.begin(), orders_all.end()));

  // From best permutation, form batches sequentially (abiding to capacity)
  std::list<std::list<std::vector<int> *> *> batches_new;
  int curr_capacity;
  for (auto order : orders_best) {
    // Check if order fits in current batch
    if (batches_new.empty() || curr_capacity + order->size() > PIptr_->getMaxCapacity()) {
      // Create new batch
      curr_capacity = order->size();
      std::list<std::vector<int> *> * batch_new = new std::list<std::vector<int> *>();
      batch_new->push_back(order);
      batches_new.push_back(batch_new);
    } else {
      // Add to last batch
      batches_new.back()->push_back(order);
      curr_capacity += order->size();
    }
  }
  // Update PI with new solution
  std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
  for (auto batch : batches) {
    batch->clear();
  }
  // Remove all batches
  worker->clearWorker();
  // Add new batches
  for (auto batch : batches_new) {
    worker->appendBatch(batch);
  }
  // Compute performance
  double perf = PIptr_->newComputeFatiguePerformance(true);
  std::cout << i << ": " << perf << std::endl;
  // update best_sol_
  best_sol_.clear();
  for (auto worker : PIptr_->getWorkers()) {
    Worker wnew(*worker);
    best_sol_.push_back(wnew);
  }
  best_perf_ = perf;
}