#include "operators.hpp"

Operators::Operators(PI * ptr) {
  PIptr_ = ptr;
}

Operators::~Operators(void) {}

void Operators::random_greedy(int q) {
  // Pop q random orders from any batch
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // // Print gen seed
  // gen.seed(1876772057);
  // std::cout << gen() << std::endl;

  std::list<std::vector<int> *> backlog;
  std::list<Worker *> workers = PIptr_->getWorkers();
  // Destruct - random
    // Random worker and batch using distribution scaled
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    std::list<std::vector<int> *> orders = **batch;
    // Random order
    int idx_order = dis(gen) % orders.size();
    auto order = std::next(orders.begin(), idx_order);
    // Add to backlog
    backlog.push_back(*order);
    // Remove from batch
    orders.erase(order);
    // Update batch
    (*batch)->clear();
    (*batch)->assign(orders.begin(), orders.end());
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - greedy
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    double best_performance = -1;
    Worker * best_worker = nullptr;
    std::list<std::vector<int> *> * best_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int i = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(i)) {
          i++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        double performance = PIptr_->newComputeFatiguePerformance();
        if (performance < best_performance || best_performance == -1) {
          best_performance = performance;
          best_worker = worker;
          best_batch = batch;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        batch->pop_back();
        i++;
      }
    }
    if (best_performance == -1) {
      // put back in idx_worker, idx_batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      best_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(best_worker, best_worker->getBatchId(best_batch), true);
    }
  }
}

void Operators::random_random(int q) {
  // Pop q random orders from any batch
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // // Print gen seed
  // std::cout << gen() << std::endl;

  std::list<std::vector<int> *> backlog;
  std::list<Worker *> workers = PIptr_->getWorkers();
  // Destruct - random
  for (int i = 0; i < q; i++) {
    // Random worker and batch
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    std::list<std::vector<int> *> orders = **batch;
    // Random order
    int idx_order = dis(gen) % orders.size();
    auto order = std::next(orders.begin(), idx_order);
    // Add to backlog
    backlog.push_back(*order);
    // Remove from batch
    orders.erase(order);
    // Update batch
    (*batch)->clear();
    (*batch)->assign(orders.begin(), orders.end());
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  }
  // Repair - random
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Append order to random batch of a random worker
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    (*batch)->push_back(order);
    // Set as changed and recompute performance for feasibility check
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    PIptr_->newComputeFatiguePerformance();
    // Reselect batch if full
    while (!PIptr_->checkFeasibility()) {
      (*batch)->pop_back();
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed and recompute performance for feasibility check
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      PIptr_->newComputeFatiguePerformance();
    }
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  }
}


void Operators::worst_greedy(int q) {
  // Remove q worst orders and insert them in the best position
  // Pop the q orders that allow for the greatest delta in performance
  std::list<std::vector<int> *> backlog;
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  // Destruct - worst
  
    // Find worst order
    double biggest_delta = -1;
    std::vector<int> * worst_order = nullptr;
    Worker * worst_worker = nullptr;
    std::list<std::vector<int> *> * worst_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      for (auto batch : worker->getBatches()) {
        for (int j = 0; j < batch->size(); j++) {
          // Remove order
          std::vector<int> * order = batch->front();
          batch->pop_front();
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // Compute delta
          double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
          // Update worst
          if (delta > biggest_delta) {
            biggest_delta = delta;
            worst_order = order;
            worst_worker = worker;
            worst_batch = batch;
          }
          // Reinsert order
          batch->push_back(order);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        }
      }
    }
    // Remove worst order
    worst_batch->remove(worst_order);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(worst_worker, worst_worker->getBatchId(worst_batch), true);
    // Update performance
    curr_performance = PIptr_->newComputeFatiguePerformance();
    // Add to backlog
    backlog.push_back(worst_order);
  

  // PIptr_->printBatches(true);

  // Repair - greedy
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    double best_performance = -1;
    Worker * best_worker = nullptr;
    std::list<std::vector<int> *> * best_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int i = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(i)) {
          i++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        double performance = PIptr_->newComputeFatiguePerformance();
        if (performance < best_performance || best_performance == -1) {
          best_performance = performance;
          best_worker = worker;
          best_batch = batch;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        batch->pop_back();
        i++;
      }
    }
    if (best_performance == -1) {
      // put back in original worker, batch
      worst_batch->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker(worst_worker, worst_worker->getBatchId(worst_batch), true);    
    } else {
      best_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(best_worker, best_worker->getBatchId(best_batch), true);
    }
  }
}

void Operators::worst_random(int q) {
  // Remove q worst orders and insert them in the best position
  // Pop the q orders that allow for the greatest delta in performance
  std::list<std::vector<int> *> backlog;
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  // Destruct - worst
  for (int i = 0; i < q; i++) {
    // Find worst order
    double biggest_delta = -1;
    std::vector<int> * worst_order = nullptr;
    Worker * worst_worker = nullptr;
    std::list<std::vector<int> *> * worst_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      for (auto batch : worker->getBatches()) {
        for (int j = 0; j < batch->size(); j++) {
          // Remove order
          std::vector<int> * order = batch->front();
          batch->pop_front();
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // Compute delta
          double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
          // Update worst
          if (delta > biggest_delta) {
            biggest_delta = delta;
            worst_order = order;
            worst_worker = worker;
            worst_batch = batch;
          }
          // Reinsert order
          batch->push_back(order);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        }
      }
    }
    // Remove worst order
    worst_batch->remove(worst_order);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(worst_worker, worst_worker->getBatchId(worst_batch), true);
    // Update performance
    curr_performance = PIptr_->newComputeFatiguePerformance();
    // Add to backlog
    backlog.push_back(worst_order);
  }

  // PIptr_->printBatches(true);

  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  std::list<Worker *> workers = PIptr_->getWorkers();
  // Repair - random
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Append order to random batch of a random worker
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    (*batch)->push_back(order);
    // Set as changed and recompute performance for feasibility check
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    PIptr_->newComputeFatiguePerformance();
    // Reselect batch if full
    while (!PIptr_->checkFeasibility()) {
      (*batch)->pop_back();
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed and recompute performance for feasibility check
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      PIptr_->newComputeFatiguePerformance();
    }
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  }
}

void Operators::Fmax_greedy() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove random order
  std::list<std::vector<int> *> orders = **batch;
  int idx_order = dis(gen) % orders.size();
  auto order = std::next(orders.begin(), idx_order);
  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - greedy
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    double best_performance = -1;
    Worker * best_worker = nullptr;
    std::list<std::vector<int> *> * best_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int i = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(i)) {
          i++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        double performance = PIptr_->newComputeFatiguePerformance();
        if (performance < best_performance || best_performance == -1) {
          best_performance = performance;
          best_worker = worker;
          best_batch = batch;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        batch->pop_back();
        i++;
      }
    }
    if (best_performance == -1) {
      // put back in idx_worker, idx_batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      best_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(best_worker, best_worker->getBatchId(best_batch), true);
    }
  }
}

void Operators::Fmax_random() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove random order
  std::list<std::vector<int> *> orders = **batch;
  int idx_order = dis(gen) % orders.size();
  auto order = std::next(orders.begin(), idx_order);
  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - random
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Append order to random batch of a random worker
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    (*batch)->push_back(order);
    // Set as changed and recompute performance for feasibility check
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    PIptr_->newComputeFatiguePerformance();
    // Reselect batch if full
    while (!PIptr_->checkFeasibility()) {
      (*batch)->pop_back();
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed and recompute performance for feasibility check
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      PIptr_->newComputeFatiguePerformance();
    }
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  }

}

void Operators::Fmax_greedy_w_pen() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove order with highest avg penibility
  std::list<std::vector<int> *> orders = **batch;
  double max_avg_pen = -1;
  auto order = orders.begin();
  for (int idx_order = 0; idx_order < orders.size(); idx_order++) {
    auto curr_order = std::next(orders.begin(), idx_order);
    // std::cout << "order avg pen: " << PIptr_->getAvgPenibility(*curr_order, *worker) << std::endl;
    if (PIptr_->getAvgPenibility(*curr_order, *worker) > max_avg_pen) {
      max_avg_pen = PIptr_->getAvgPenibility(*order, *worker);
      order = std::next(orders.begin(), idx_order);
    }
  }

  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - greedy
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    double best_performance = -1;
    Worker * best_worker = nullptr;
    std::list<std::vector<int> *> * best_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int i = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(i)) {
          i++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        double performance = PIptr_->newComputeFatiguePerformance();
        if (performance < best_performance || best_performance == -1) {
          best_performance = performance;
          best_worker = worker;
          best_batch = batch;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        batch->pop_back();
        i++;
      }
    }
    if (best_performance == -1) {
      // put back in idx_worker, idx_batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      best_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(best_worker, best_worker->getBatchId(best_batch), true);
    }
  }
}


void Operators::Fmax_random_w_pen() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove order with highest avg penibility
  std::list<std::vector<int> *> orders = **batch;
  double max_avg_pen = -1;
  auto order = orders.begin();
  for (int idx_order = 0; idx_order < orders.size(); idx_order++) {
    auto curr_order = std::next(orders.begin(), idx_order);
    // std::cout << "order avg pen: " << PIptr_->getAvgPenibility(*curr_order, *worker) << std::endl;
    if (PIptr_->getAvgPenibility(*curr_order, *worker) > max_avg_pen) {
      max_avg_pen = PIptr_->getAvgPenibility(*order, *worker);
      order = std::next(orders.begin(), idx_order);
    }
  }

  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - random
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Append order to random batch of a random worker
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    (*batch)->push_back(order);
    // Set as changed and recompute performance for feasibility check
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    PIptr_->newComputeFatiguePerformance();
    // Reselect batch if full
    while (!PIptr_->checkFeasibility()) {
      (*batch)->pop_back();
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed and recompute performance for feasibility check
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
      PIptr_->newComputeFatiguePerformance();
    }
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  }
}

void Operators::random_Flow() {
  // Pop 1 random orders from any batch
  int q = 1;
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // // Print gen seed
  // gen.seed(1876772057);
  // std::cout << gen() << std::endl;

  std::list<std::vector<int> *> backlog;
  std::list<Worker *> workers = PIptr_->getWorkers();
  // Destruct - random
  
    // Random worker and batch using distribution scaled
    int idx_worker = dis(gen) % workers.size();
    auto worker = std::next(workers.begin(), idx_worker);
    std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
    int idx_batch = dis(gen) % batches.size();
    auto batch = std::next(batches.begin(), idx_batch);
    // Reselect batch if empty
    while ((*batch)->empty() || batches.size() < 1) {
      idx_worker = dis(gen) % workers.size();
      worker = std::next(workers.begin(), idx_worker);
      batches = (*worker)->getBatches();
      idx_batch = dis(gen) % batches.size();
      batch = std::next(batches.begin(), idx_batch);
    }
    std::list<std::vector<int> *> orders = **batch;
    // Random order
    int idx_order = dis(gen) % orders.size();
    auto order = std::next(orders.begin(), idx_order);
    // Add to backlog
    backlog.push_back(*order);
    // Remove from batch
    orders.erase(order);
    // Update batch
    (*batch)->clear();
    (*batch)->assign(orders.begin(), orders.end());
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  
  // PIptr_->printBatches(true);

  // Repair - insert in batch with lowest Favg
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Find batch with lowest Favg
    double lowest_favg = -1;
    Worker * lowest_worker = nullptr;
    std::list<std::vector<int> *> * lowest_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int batch_id = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(batch_id)) {
          batch_id++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        double favg = worker->getAvgFatigue(batch_id);
        if (favg < lowest_favg || lowest_favg == -1) {
          lowest_favg = favg;
          lowest_worker = worker;
          lowest_batch = batch;
        }
        batch_id++;
        batch->pop_back();
      }
    }
    if (lowest_favg == -1) {
      // put back in original worker, batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      lowest_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(lowest_worker, lowest_worker->getBatchId(lowest_batch), true);
    }
  }
}

void Operators::worst_Flow() {
  int q = 1;
  // Pop the q orders that allow for the greatest delta in performance
  std::list<std::vector<int> *> backlog;
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  // Destruct - worst
  
    // Find worst order
    double biggest_delta = -1;
    std::vector<int> * worst_order = nullptr;
    Worker * worst_worker = nullptr;
    std::list<std::vector<int> *> * worst_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      for (auto batch : worker->getBatches()) {
        for (int j = 0; j < batch->size(); j++) {
          // Remove order
          std::vector<int> * order = batch->front();
          batch->pop_front();
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // Compute delta
          double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
          // Update worst
          if (delta > biggest_delta) {
            biggest_delta = delta;
            worst_order = order;
            worst_worker = worker;
            worst_batch = batch;
          }
          // Reinsert order
          batch->push_back(order);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        }
      }
    }
    // Remove worst order
    worst_batch->remove(worst_order);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(worst_worker, worst_worker->getBatchId(worst_batch), true);
    // Update performance
    curr_performance = PIptr_->newComputeFatiguePerformance();
    // Add to backlog
    backlog.push_back(worst_order);
  

  // PIptr_->printBatches(true);

  // Repair - insert in batch with lowest Favg
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Find batch with lowest Favg
    double lowest_favg = -1;
    Worker * lowest_worker = nullptr;
    std::list<std::vector<int> *> * lowest_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int batch_id = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(batch_id)) {
          batch_id++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        double favg = worker->getAvgFatigue(batch_id);
        if (favg < lowest_favg || lowest_favg == -1) {
          lowest_favg = favg;
          lowest_worker = worker;
          lowest_batch = batch;
        }
        batch_id++;
        batch->pop_back();
      }
    }
    if (lowest_favg == -1) {
      // put back in original worker, batch
      worst_batch->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker(worst_worker, worst_worker->getBatchId(worst_batch), true);    
    } else {
      lowest_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(lowest_worker, lowest_worker->getBatchId(lowest_batch), true);
    }
  }
}

void Operators::Fmax_Flow() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove random order
  std::list<std::vector<int> *> orders = **batch;
  int idx_order = dis(gen) % orders.size();
  auto order = std::next(orders.begin(), idx_order);
  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  // Repair - insert in batch with lowest Favg
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Find batch with lowest Favg
    double lowest_favg = -1;
    Worker * lowest_worker = nullptr;
    std::list<std::vector<int> *> * lowest_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int batch_id = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(batch_id)) {
          batch_id++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        double favg = worker->getAvgFatigue(batch_id);
        if (favg < lowest_favg || lowest_favg == -1) {
          lowest_favg = favg;
          lowest_worker = worker;
          lowest_batch = batch;
        }
        batch_id++;
        batch->pop_back();
      }
    }
    if (lowest_favg == -1) {
      // put back in idx_worker, idx_batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      lowest_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(lowest_worker, lowest_worker->getBatchId(lowest_batch), true);
    }
  }
}

void Operators::Fmax_Flow_w_pen() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);

  // Remove random order from batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<std::vector<int> *> backlog;

  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove order with highest avg penibility
  std::list<std::vector<int> *> orders = **batch;
  double max_avg_pen = -1;
  auto order = orders.begin();
  for (int idx_order = 0; idx_order < orders.size(); idx_order++) {
    auto curr_order = std::next(orders.begin(), idx_order);
    // std::cout << "order avg pen: " << PIptr_->getAvgPenibility(*curr_order, *worker) << std::endl;
    if (PIptr_->getAvgPenibility(*curr_order, *worker) > max_avg_pen) {
      max_avg_pen = PIptr_->getAvgPenibility(*order, *worker);
      order = std::next(orders.begin(), idx_order);
    }
  }

  // Add to backlog
  backlog.push_back(*order);
  // Remove from batch
  orders.erase(order);
  // Update batch
  (*batch)->clear();
  (*batch)->assign(orders.begin(), orders.end());
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // Repair - insert in batch with lowest Favg
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    // Find batch with lowest Favg
    double lowest_favg = -1;
    Worker * lowest_worker = nullptr;
    std::list<std::vector<int> *> * lowest_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      int batch_id = 0;
      for (auto batch : worker->getBatches()) {
        // Skip empty
        if (worker->batchIsEmpty(batch_id)) {
          batch_id++;
          continue;
        }
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set as changed and recompute performance for feasibility check
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          // PIptr_->newComputeFatiguePerformance();
          continue;
        }
        double favg = worker->getAvgFatigue(batch_id);
        if (favg < lowest_favg || lowest_favg == -1) {
          lowest_favg = favg;
          lowest_worker = worker;
          lowest_batch = batch;
        }
        batch_id++;
        batch->pop_back();
      }
    }
    if (lowest_favg == -1) {
      // put back in idx_worker, idx_batch
      auto worker = std::next(workers.begin(), idx_worker);
      auto batch = std::next(batches.begin(), idx_batch);
      (*batch)->push_back(order);
      // Set as changed
      PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
    } else {
      lowest_batch->push_back(order);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(lowest_worker, lowest_worker->getBatchId(lowest_batch), true);
    }
  }
}

void Operators::b_random_random() {
  std::random_device rd;
  // std::mt19937 gen(rd());
  std::mt19937 gen(4);
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // // Print seed
  // std::cout << "Fixed seed " << gen() << std::endl;

  // Remove a random batch from a random worker
  std::list<Worker *> workers = PIptr_->getWorkers();
  int idx_worker = dis(gen) % workers.size();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  int idx_batch = dis(gen) % batches.size();
  auto batch = std::next(batches.begin(), idx_batch);
  // Reselect batch if empty
  while ((*batch)->empty() || batches.size() < 1) {
    idx_worker = dis(gen) % workers.size();
    worker = std::next(workers.begin(), idx_worker);
    batches = (*worker)->getBatches();
    idx_batch = dis(gen) % batches.size();
    batch = std::next(batches.begin(), idx_batch);
  }
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  PIptr_->newComputeFatiguePerformance();

  // PIptr_->printBatches(true);

  // Insert the batch in a random worker after a random batch
  int idx_worker2 = dis(gen) % workers.size();
  auto worker2 = std::next(workers.begin(), idx_worker2);
  std::list<std::list<std::vector<int> *> *> batches2 = (*worker2)->getBatches();
  // Nr between 0 and size of batches + 1 
  int idx_batch2 = dis(gen) % (batches2.size() + 1);
  // If batch at idx_batch2 is empty, reselect it
  while ((*worker2)->getBatchesTimeFend(idx_batch2).first == 0) {
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
  }
  (*worker2)->insertBatch(idx_batch2, temp);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
  PIptr_->newComputeFatiguePerformance();
  // Check feasibility
  while (!PIptr_->checkFeasibility()) {
    (*worker2)->removeBatch(idx_batch2);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // New position for insertion
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
    (*worker2)->insertBatch(idx_batch2, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // Compute performance for next feasibility check
    PIptr_->newComputeFatiguePerformance();
  }
}

void Operators::b_random_greedy() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Remove a random batch from a random worker
  std::list<Worker *> workers = PIptr_->getWorkers();
  int idx_worker = dis(gen) % workers.size();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  int idx_batch = dis(gen) % batches.size();
  auto batch = std::next(batches.begin(), idx_batch);
  // Reselect batch if empty
  while ((*batch)->empty() || batches.size() < 1) {
    idx_worker = dis(gen) % workers.size();
    worker = std::next(workers.begin(), idx_worker);
    batches = (*worker)->getBatches();
    idx_batch = dis(gen) % batches.size();
    batch = std::next(batches.begin(), idx_batch);
  }
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch in a the best position (greedy)
  double best_performance = -1;
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size() + 1; idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      // Try insert
      worker->insertBatch(idx_batch, temp);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
      // Compute performance
      double performance = PIptr_->newComputeFatiguePerformance();
      if (!PIptr_->checkFeasibility()) {
        worker->removeBatch(idx_batch);
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance for next feasibility check
        PIptr_->newComputeFatiguePerformance();
        continue;
      }
      if (performance < best_performance || best_performance == -1) {
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
      }
      worker->removeBatch(idx_batch);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
    }
  }
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    (*worker)->insertBatch(idx_batch, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  } else {
    best_worker->insertBatch(best_batch_id, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id, true);
  }
}

void Operators::b_random_Fend() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Remove a random batch from a random worker
  std::list<Worker *> workers = PIptr_->getWorkers();
  int idx_worker = dis(gen) % workers.size();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  int idx_batch = dis(gen) % batches.size();
  auto batch = std::next(batches.begin(), idx_batch);
  // Reselect batch if empty
  while ((*batch)->empty() || batches.size() < 1) {
    idx_worker = dis(gen) % workers.size();
    worker = std::next(workers.begin(), idx_worker);
    batches = (*worker)->getBatches();
    idx_batch = dis(gen) % batches.size();
    batch = std::next(batches.begin(), idx_batch);
  }
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch after the batch with lowest Fend
  double best_performance = -1;  // in terms of lowest Fend
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size(); idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      double performance = worker->getBatchesTimeFend(idx_batch).second;
      if (performance < best_performance || best_performance == -1) {
        // Try insertion and see if it is feasible
        worker->insertBatch(idx_batch + 1, temp);  // + 1 for insertin after idx_batch
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance
        double performance = PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility() || worker->batchIsEmpty(idx_batch)) {
          worker->removeBatch(idx_batch + 1);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
          // Compute performance for next feasibility check
          PIptr_->newComputeFatiguePerformance();
          continue;
        }
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        worker->removeBatch(idx_batch + 1);
      }
    }
  }
  // If still -1, insert in original position
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    (*worker)->insertBatch(idx_batch, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  } else {
    best_worker->insertBatch(best_batch_id + 1, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id + 1, true);
  }
}


void Operators::b_worst_random() {
  // Remove the batch that gives the largest savings
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  double biggest_delta = -1;
  Worker * worst_worker = nullptr;
  int worst_batch_id = -1;
  std::list<std::vector<int> *> worst_batch;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    int batch_id = 0;
    for (auto batch : batches) {
      // If empty continue
      if (batch->empty()) {
        batch_id++;
        continue;
      }
      // Remove batch
      std::list<std::vector<int> *> temp = *batch;
      worker->removeBatch(batch_id);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      // Compute delta
      double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
      if (delta > biggest_delta) {
        biggest_delta = delta;
        worst_worker = worker;
        worst_batch_id = batch_id;
        worst_batch = temp;
      }
      // Reinsert batch
      worker->insertBatch(batch_id, temp);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      batch_id++;
    }
  }
  // Remove worst batch
  worst_worker->removeBatch(worst_batch_id);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker(worst_worker, worst_batch_id, true);

  // PIptr_->printBatches(true);

  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Insert the batch in a random worker after a random batch
  std::list<Worker *> workers = PIptr_->getWorkers();
  int idx_worker2 = dis(gen) % workers.size();
  auto worker2 = std::next(workers.begin(), idx_worker2);
  std::list<std::list<std::vector<int> *> *> batches2 = (*worker2)->getBatches();
  // Nr between 0 and size of batches + 1 
  int idx_batch2 = dis(gen) % (batches2.size() + 1);
  // If batch at idx_batch2 is empty, reselect it
  while ((*worker2)->getBatchesTimeFend(idx_batch2).first == 0) {
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
  }
  (*worker2)->insertBatch(idx_batch2, worst_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
  PIptr_->newComputeFatiguePerformance();
  // Check feasibility
  while (!PIptr_->checkFeasibility()) {
    (*worker2)->removeBatch(idx_batch2);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // New position for insertion
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
    (*worker2)->insertBatch(idx_batch2, worst_batch);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // Compute performance for next feasibility check
    PIptr_->newComputeFatiguePerformance();
  }
}

void Operators::b_worst_greedy() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Remove the batch that gives the largest savings
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  double biggest_delta = -1;
  Worker * worst_worker = nullptr;
  int worst_batch_id = -1;
  std::list<std::vector<int> *> worst_batch;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    int batch_id = 0;
    for (auto batch : batches) {
      // If empty continue
      if (batch->empty()) {
        batch_id++;
        continue;
      }
      // Remove batch
      std::list<std::vector<int> *> temp = *batch;
      worker->removeBatch(batch_id);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      // Compute delta
      double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
      if (delta > biggest_delta) {
        biggest_delta = delta;
        worst_worker = worker;
        worst_batch_id = batch_id;
        worst_batch = temp;
      }
      // Reinsert batch
      worker->insertBatch(batch_id, temp);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      batch_id++;
    }
  }
  // Remove worst batch
  worst_worker->removeBatch(worst_batch_id);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker(worst_worker, worst_batch_id, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch in a the best position (greedy)
  double best_performance = -1;
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size() + 1; idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      // Try insert
      worker->insertBatch(idx_batch, worst_batch);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
      // Compute performance
      double performance = PIptr_->newComputeFatiguePerformance();
      if (!PIptr_->checkFeasibility()) {
        worker->removeBatch(idx_batch);
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance for next feasibility check
        PIptr_->newComputeFatiguePerformance();
        continue;
      }
      if (performance < best_performance || best_performance == -1) {
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
      }
      worker->removeBatch(idx_batch);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
    }
  }
  // If still -1, insert in original position
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    worst_worker->insertBatch(worst_batch_id, worst_batch);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(worst_worker, worst_batch_id, true);
  } else {
    best_worker->insertBatch(best_batch_id, worst_batch);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id, true);
  }
}

void Operators::b_worst_Fend() {
  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Remove the batch that gives the largest savings
  double curr_performance = PIptr_->newComputeFatiguePerformance();
  double biggest_delta = -1;
  Worker * worst_worker = nullptr;
  int worst_batch_id = -1;
  std::list<std::vector<int> *> worst_batch;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    int batch_id = 0;
    for (auto batch : batches) {
      // If empty continue
      if (batch->empty()) {
        batch_id++;
        continue;
      }
      // Remove batch
      std::list<std::vector<int> *> temp = *batch;
      worker->removeBatch(batch_id);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      // Compute delta
      double delta = curr_performance - PIptr_->newComputeFatiguePerformance();
      if (delta > biggest_delta) {
        biggest_delta = delta;
        worst_worker = worker;
        worst_batch_id = batch_id;
        worst_batch = temp;
      }
      // Reinsert batch
      worker->insertBatch(batch_id, temp);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, batch_id, true);
      batch_id++;
    }
  }
  // Remove worst batch
  worst_worker->removeBatch(worst_batch_id);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker(worst_worker, worst_batch_id, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch after the batch with lowest Fend
  double best_performance = -1;  // in terms of lowest Fend
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size(); idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      double performance = worker->getBatchesTimeFend(idx_batch).second;
      if (performance < best_performance || best_performance == -1) {
        // Try insertion and see if it is feasible
        worker->insertBatch(idx_batch + 1, worst_batch);  // + 1 for insertin after idx_batch
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance
        double performance = PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility() || worker->batchIsEmpty(idx_batch)) {
          worker->removeBatch(idx_batch + 1);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
          // Compute performance for next feasibility check
          PIptr_->newComputeFatiguePerformance();
          continue;
        }
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        worker->removeBatch(idx_batch + 1);
      }
    }
  }
  // If still -1, insert in original position
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    worst_worker->insertBatch(worst_batch_id, worst_batch);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(worst_worker, worst_batch_id, true);
  } else {
    best_worker->insertBatch(best_batch_id + 1, worst_batch);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id + 1, true);
  }
}

void Operators::b_Fmax_random() {
  // Identify the batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  
  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // PIptr_->printBatches(true);

  std::random_device rd;
  std::mt19937 gen(rd());
  // Uniform distribution
  std::uniform_int_distribution<> dis(0, 100);
  // Insert the batch in a random worker after a random batch
  int idx_worker2 = dis(gen) % workers.size();
  auto worker2 = std::next(workers.begin(), idx_worker2);
  std::list<std::list<std::vector<int> *> *> batches2 = (*worker2)->getBatches();
  // Nr between 0 and size of batches + 1 
  int idx_batch2 = dis(gen) % (batches2.size() + 1);
  // If batch at idx_batch2 is empty, reselect it
  while ((*worker2)->getBatchesTimeFend(idx_batch2).first == 0) {
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
  }
  (*worker2)->insertBatch(idx_batch2, temp);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
  PIptr_->newComputeFatiguePerformance();
  // Check feasibility
  while (!PIptr_->checkFeasibility()) {
    (*worker2)->removeBatch(idx_batch2);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // New position for insertion
    idx_worker2 = dis(gen) % workers.size();
    worker2 = std::next(workers.begin(), idx_worker2);
    batches2 = (*worker2)->getBatches();
    idx_batch2 = dis(gen) % (batches2.size() + 1);
    (*worker2)->insertBatch(idx_batch2, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker2), idx_batch2, true);
    // Compute performance for next feasibility check
    PIptr_->newComputeFatiguePerformance();
  }
}

void Operators::b_Fmax_greedy() {
  // Identify the batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch in a the best position (greedy)
  double best_performance = -1;
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size() + 1; idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      // Try insert
      worker->insertBatch(idx_batch, temp);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
      // Compute performance
      double performance = PIptr_->newComputeFatiguePerformance();
      if (!PIptr_->checkFeasibility()) {
        worker->removeBatch(idx_batch);
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance for next feasibility check
        PIptr_->newComputeFatiguePerformance();
        continue;
      }
      if (performance < best_performance || best_performance == -1) {
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
      }
      worker->removeBatch(idx_batch);
      // Set batch as changed
      PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
    }
  }
  // If still -1, insert in original position
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    (*worker)->insertBatch(idx_batch, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  } else {
    best_worker->insertBatch(best_batch_id, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id, true);
  }
}

void Operators::b_Fmax_Fend() {
  // Identify the batch with highest Fend
  int idx_worker;
  int idx_batch;
  PIptr_->newComputeFatiguePerformance();
  std::tie(idx_worker, idx_batch) = PIptr_->getFmaxWorkerBatchId();
  std::list<Worker *> workers = PIptr_->getWorkers();
  auto worker = std::next(workers.begin(), idx_worker);
  std::list<std::list<std::vector<int> *> *> batches = (*worker)->getBatches();
  auto batch = std::next(batches.begin(), idx_batch);
  // Remove batch
  std::list<std::vector<int> *> temp = **batch;
  (*worker)->removeBatch(idx_batch);
  // Set batch as changed
  PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);

  // PIptr_->printBatches(true);

  // Insert the entire batch after the batch with lowest Fend
  double best_performance = -1;  // in terms of lowest Fend
  Worker * best_worker = nullptr;
  int best_batch_id = -1;
  for (auto worker : PIptr_->getWorkers()) {
    std::list<std::list<std::vector<int> *> *> batches = worker->getBatches();
    for (int idx_batch = 0; idx_batch < batches.size(); idx_batch++) {
      // Skip empty batches
      if (worker->getBatchesTimeFend(idx_batch).first == 0) {
        continue;
      }
      double performance = worker->getBatchesTimeFend(idx_batch).second;
      if (performance < best_performance || best_performance == -1) {
        // Try insertion and see if it is feasible
        worker->insertBatch(idx_batch + 1, temp);  // + 1 for insertin after idx_batch
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        // Compute performance
        double performance = PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility() || worker->batchIsEmpty(idx_batch)) {
          worker->removeBatch(idx_batch + 1);
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
          // Compute performance for next feasibility check
          PIptr_->newComputeFatiguePerformance();
          continue;
        }
        best_performance = performance;
        best_worker = worker;
        best_batch_id = idx_batch;
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, idx_batch, true);
        worker->removeBatch(idx_batch + 1);
      }
    }
  }
  // If still -1, insert in original position
  if (best_batch_id == -1) {
    // std::cout << "case 0" << std::endl;
    (*worker)->insertBatch(idx_batch, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker((*worker), idx_batch, true);
  } else {
    best_worker->insertBatch(best_batch_id + 1, temp);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_batch_id + 1, true);
  }
}

void Operators::execute(int idx_op, int q) {
  switch (idx_op) {
    case 0:
      random_greedy(q);
      break;
    case 1:
      random_random(q);
      break;
    case 2:
      worst_greedy(q);
      break;
    case 3:
      worst_random(q);
      break;
    case 4:
      Fmax_greedy();
      break;
    case 5:
      Fmax_random();
      break;
    case 6:
      Fmax_greedy_w_pen();
      break;
    case 7:
      Fmax_random_w_pen();
      break;
    case 8:
      random_Flow();
      break;
    case 9:
      worst_Flow();
      break;
    case 10:
      Fmax_Flow();
      break;
    case 11:
      Fmax_Flow_w_pen();
      break;
    case 12:
      b_random_random();
      break;
    case 13:
      b_random_greedy();
      break;
    case 14:
      b_random_Fend();
      break;
    case 15:
      b_worst_random();
      break;
    case 16:
      b_worst_greedy();
      break;
    case 17:
      b_worst_Fend();
      break;
    case 18:
      b_Fmax_random();
      break;
    case 19:
      b_Fmax_greedy();
      break;
    case 20:
      b_Fmax_Fend();
      break;
    case 21:
      empty_greedy();
      break;
    default:
      break;
  }
}

void Operators::empty_greedy() {
  // identify the Worker with highest average fatigue level
  std::list<Worker *> workers = PIptr_->getWorkers();
  Worker * worst_worker = nullptr;
  double worst_fatigue = -1;
  for (auto worker : workers) {
    double fatigue = worker->getAvgFatigue();
    // std::cout << fatigue << std::endl;
    if (fatigue > worst_fatigue) {
      worst_fatigue = fatigue;
      worst_worker = worker;
    }
  }
  // std::cout << "Worst worker: " << worst_worker->getParams()[0] << " with fatigue: " << worst_fatigue << std::endl;

  // Backlog of all order assigned to worst_worker
  std::list<std::vector<int> *> backlog;
  std::list<std::list<std::vector<int> *> *> batches = worst_worker->getBatches();
  for (auto batch : batches) {
    for (auto order : *batch) {
      backlog.push_back(order);
    }
  }
  // Clear all batches of worst_worker
  for (auto batch : batches) {
    batch->clear();
  }
  // Set all batches as changed
  for (int i = 0; i < batches.size(); i++) {
    PIptr_->updateChangedBatchForWorker(worst_worker, i, true);
  }

  // PIptr_->printBatches(true);

  // Repair - greedy
  while (!backlog.empty()) {
    std::vector<int> * order = backlog.front();
    backlog.pop_front();
    double best_performance = -1;
    Worker * best_worker = nullptr;
    std::list<std::vector<int> *> * best_batch = nullptr;
    for (auto worker : PIptr_->getWorkers()) {
      for (auto batch : worker->getBatches()) {
        // dont skip empty batch for dealing with the case when all other batches are full
        batch->push_back(order);
        // Set as changed and recompute performance for feasibility check
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        PIptr_->newComputeFatiguePerformance();
        if (!PIptr_->checkFeasibility()) {
          batch->pop_back();
          // Set batch as changed
          PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
          continue;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        double performance = PIptr_->newComputeFatiguePerformance();
        if (performance < best_performance || best_performance == -1) {
          best_performance = performance;
          best_worker = worker;
          best_batch = batch;
        }
        // Set batch as changed
        PIptr_->updateChangedBatchForWorker(worker, worker->getBatchId(batch), true);
        batch->pop_back();
      }
    }
    best_batch->push_back(order);
    // Set batch as changed
    PIptr_->updateChangedBatchForWorker(best_worker, best_worker->getBatchId(best_batch), true);
  }
}
