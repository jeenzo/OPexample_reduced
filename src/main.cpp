#include "warehouse.hpp"
#include "inputs.hpp"
#include "routing.hpp"
#include "problem_instance.hpp"
#include "operators.hpp"
#include "alns.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>


void usage_instructions() {
  std::cout << "Usage: ./JOBASP-f [options]\n";
  std::cout << "Options:\n";
  std::cout << "\t--help\t\t\tShow this message\n";
  std::cout << "\t--iter\t\t\tSpecify number of iterations (per segment)\n";
  std::cout << "\t--seg\t\t\tSpecify number of segments\n";
  std::cout << "\t--cap\t\t\tSpecify max batch capacity [items]\n";
  std::cout << "\t--shift\t\t\tSpecify shift time [h]\n";
  std::cout << "\t--in <filename>\t\t(IN) Specify folder for instance files\n";
  std::cout << "\t--out <filename>\t\t(OUT) Specify output file(s) namescheme\n";
  std::cout << "\t--N \t\t\t(IN) Specify number of orders of input file for reading\n";
  std::cout << "\t--repl \t\t\t(IN) Specify the replication number\n";
  std::cout << "\t--trackIt\t\t\tSave best and curr performance at each iteration\n";
  std::cout << "\t--cool\t\t\tSpecify cooling rate\n";
  std::cout << "\t--wait\t\t\tfor k iter between shakes\n";
  std::cout << "\t--Y\t\t\tfor k iter accept every solution\n";
  std::cout << "\t--shakeT\t\tTemperatire rise after shake\n";
  std::cout << "\t--shakec\t\tCooling rate after shake\n";
  std::cout << "\t--trueC\t\tActual batch capacity\n";
  std::cout << "\t --eval \t\t\tResult specific jolly parameter\n";
  std::cout << "\t--workers\t\t\t idx of file for worker params\n";
}


int main(int argc, char* argv[]) {
  /* ------------------------ BASELINE INPUTS ------------------------ */
  // Input folder path
  std::string input_folder = "../data/instances/";
  // Input file names
  std::string wh_file = "warehouse_data.csv";
  std::string orders_file = "orders.csv";
  std::string pen_file = "penibility_matrix.csv";
  std::string time_file = "task_time_matrix.csv";
  std::string worker_file = "worker_params.csv";
  std::string out_namescheme = "_";
  std::string name_saving;
  // Standard instance folder
  std::string instance_folder = "class_based";
  // Standard parameters
  int batch_capacity = 12;
  double shift_time = 4;  // like in Zhuan 2022
  int actual_batch_capacity = batch_capacity;
  // ALNS parameters
  int iterations = 500;
  int segments = 200;
  int Qmax = 1;
  double reset_at = 210; // old
  // Benchmark files parameters
  int n_orders;
  int repl = 0;
  // Outputs
  bool track_iter = false;
  // Testing shake parameters
  double coolrate = 0.9999;
  int wait = 15000;
  int Y = 0;
  double shake_T_multiplier = 2;
  double shake_coolrate = 0.999;
  // Operator evaluation: 
  int k = -1;
  // Res
  int jolly = 1;
  int w_Fmax = -2;

  // Get inputs from user as per usage_instructions()
  std::string arg;
  for (int i = 1; i < argc; i++) {
    arg = argv[i];
    if (arg == "--help") {
      usage_instructions();
      return 0;
    } else if (arg == "--iter") {
      iterations = std::stoi(argv[++i]);
    } else if (arg == "--seg") {
      segments = std::stoi(argv[++i]);
      reset_at = segments + 10;
    } else if (arg == "--cap") {
      batch_capacity = std::stoi(argv[++i]);
      actual_batch_capacity = batch_capacity;
    } else if (arg == "--shift") {
      shift_time = std::stod(argv[++i]);
    } else if (arg == "--in") {
      instance_folder = argv[++i];
      // If benchmark, set file path for order files
      if (instance_folder == "bm_un") {
        orders_file = "orders_un_";
      } else if (instance_folder == "bm_la") {
        orders_file = "orders_la_";
      } else if (instance_folder == "bm_cb") {
        orders_file = "orders_cb_";
      }
    } else if (arg == "--out") {
      out_namescheme = argv[++i];
    } else if (arg == "--N") {
      n_orders = std::stoi(argv[++i]);
      orders_file += 'n' + std::to_string(n_orders) + '_';
    } else if (arg == "--repl") {
      repl = std::stoi(argv[++i]);
    } else if (arg == "--trackIt") {
      track_iter = true;
    } else if (arg == "--cool") {
      coolrate = std::stod(argv[++i]);
    } else if (arg == "--wait") {
      wait = std::stoi(argv[++i]);
    } else if (arg == "--Y") {
      Y = std::stoi(argv[++i]);
    } else if (arg == "--workers") {
      w_Fmax = std::stoi(argv[++i]);
    } else if (arg == "--shakeT") {
      shake_T_multiplier = std::stod(argv[++i]);
    } else if (arg == "--shakec") {
      shake_coolrate = std::stod(argv[++i]);
    } else if (arg == "--trueC") {
      actual_batch_capacity = std::stoi(argv[++i]);
    } else if (arg == "--eval") {
      jolly = std::stoi(argv[++i]);
    } else {
      std::cout << "Unrecognized option " << arg << std::endl;
      usage_instructions();
      return 1;
    }
  }  
  // Shift time to seconds
  shift_time *= 3600;

  /* ------------------------ BENCHMARK RUNS ------------------------ */
  if (instance_folder == "bm_un" || instance_folder == "bm_cb") {
    // General inputs
    std::string folder = "retailer1";
    input_folder += folder + "/";
    wh_file = input_folder + wh_file;
    pen_file = input_folder + "penibility_matrix.csv";
    time_file = input_folder + "task_time_matrix.csv";
    worker_file = input_folder + "worker_params.csv";
    if (jolly == 0) {
      worker_file = input_folder + "worker_paramsNoDegr.csv";
    }
    if (w_Fmax >= 0) {
      worker_file = input_folder + "worker_params_Fmax" + std::to_string(w_Fmax) + ".csv";
    } 
    // Select orders file
    name_saving = input_folder + orders_file;
    orders_file = input_folder + orders_file;
    // Check that replication nr matches with max
    int i_start = 1;
    if (instance_folder == "bm_un" || instance_folder == "bm_cb") {
      // If repl is not specified
      if (repl == 0) {
        repl = 39;
      } else {
        i_start = repl;
        if (repl > 39) {
          std::cout << "Max replication number for small benchmarks is 39.\n";
          return 1;
        }
      }
    } else if (instance_folder == "bm_la") {
      // If repl is not specified
      if (repl == 0) {
        repl = 9;
      } else {
        i_start = repl;
        if (repl > 9) {
          std::cout << "Max replication number for large benchmarks is 9.\n";
          return 1;
        }
      }
    } else {
      std::cout << "Benchmark folder not recognized.\n";
      return 1;
    }
    if (repl < 1) {
      std::cout << "Replication number must be at least 1.\n";
      return 1;
    }
    // Replication file variables
    std::string curr_orders;
    std::string curr_namescheme;
    for (int i = i_start; i <= repl; i++) {
      // Execution time
      auto start_time = std::chrono::high_resolution_clock::now();
      // curr_namescheme
      curr_orders = orders_file + std::to_string(i);
      name_saving = name_saving + std::to_string(i);
      curr_namescheme = name_saving.substr(name_saving.find_last_of("/")+1);
      curr_namescheme = curr_namescheme.substr(9);
      curr_namescheme = instance_folder + curr_namescheme + '_';
      if (out_namescheme != "_") {
        curr_namescheme += out_namescheme;
      }
      curr_orders += ".csv";
      // std::cout << curr_namescheme << std::endl;
      InputData data = readInputs(wh_file, curr_orders, pen_file, time_file, worker_file);

      std::cout << "\n-----------Initial solution" << std::endl;
      // Workers
      std::vector<int> idx_workers = {0};
      // Initialize
      std::vector<char> function_ids = {'A', 'A', 'A', 'A'};
      // try initializing, if it fails, add to idx_workers
      int next_id = 1;
      bool fis = false;
      while (!fis) {
        while (true) {
          try {
            // std::cout << "Trying to initialize with "<< next_id << std::endl;
            PI pi(curr_namescheme, data, idx_workers, shift_time, function_ids);
            pi.initializeBatches(actual_batch_capacity);
            fis = true;
            break;
          } catch (std::exception& e) {
            // std::cout << "Not enough workers to complete all orders. Adding worker " << next_id << std::endl;
            // Add to idx_workers
            idx_workers.push_back(next_id);
            next_id++;
            // PI pi(curr_namescheme, data, idx_workers, shift_time, function_ids);
            // pi.initializeBatches(batch_capacity);
            break;
          }
        }
      }
      // Print nr. of workers
      std::cout << "Nr. of workers: " << idx_workers.size() << std::endl;

      // Output path
      std::string out_folder = folder + "B1"; // + std::to_string(actual_batch_capacity) + "-" + std::to_string(w_Fmax);
      std::string out_path = "../data/results/" + out_folder + "/";
      // Create folder if not already present
      std::filesystem::create_directories(out_path);
      std::string name = curr_namescheme;
      std::cout << name << std::endl;
      PI pi(name, data, idx_workers, shift_time, function_ids);
      pi.initializeBatches(actual_batch_capacity);
      // pi.printBatches(true);
      std::cout << "T new:\t" << pi.newComputeFatiguePerformance() << "\n";

      // // Rebuild from existing batches - A2r
      // std::cout << "\n-----------Restore a solution" << std::endl;
      // std::string input_batches = "../data/instances/resA2/";
      // input_batches = input_batches + 'n' + std::to_string(n_orders) + "_c" + std::to_string(batch_capacity) + '_' + std::to_string(repl) + "_input_batches.csv";
      // // input_batches = "../data/instances/input_batches.csv";
      // pi.rebuildBatchesFromFile(input_batches);
      // std::cout << "T new:\t" << pi.newComputeFatiguePerformance(true) << "\n";

      // Optimization (w/o saved info other than batches)
      std::cout << "\n-----------New optimization approach" << std::endl;
      // ALNS
      int Nop = 21;
      std::vector<int> op_ids = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,  // orders
        12, 13, 14, 15, 16, 17, 18, 19, 20  // batches
        // , 21  // empty-greedy
                                 };
      // If k -1 use all
      if (k != -1) {
        // Remove k from op_ids
        op_ids.erase(op_ids.begin() + k);
      }
      ALNS optimizer(&pi, Nop, op_ids);
      optimizer.run(segments, iterations,
                    {28, 9, 3},  // score1, score2, score3
                    0.1,  // reaction factor
                    0.999875,  // cooling rate
                    0.005,  // worsening factor
                    Qmax,
                    12500,  // wait
                    20,  // Y
                    0.5,  // w' shake = w *
                    0.999,  // c' shake
                    false, track_iter, out_folder);
      // End time
      auto end_time = std::chrono::high_resolution_clock::now();
      // optimizer.printSolutions();

      /* ------------------------ SAVING TO FILE ------------------------ */
      std::cout << "\n-----------Outputs" << std::endl;
      // Save operators info
      optimizer.saveOperatorTotParams(out_folder);
      // Best solution
      optimizer.updateToBest();
      pi.computeTotalPerformance();  // to get routes, fatigue, and times
      std::cout << "Saving recap: ";
      // Create file
      std::string descr_filename = out_path + name + "descr.txt";
      std::ofstream descr_file;
      descr_file.open(descr_filename);
      descr_file << "Input filepath: " << input_folder << "\n";
      descr_file << "Batch capacity: " << actual_batch_capacity << "\n";
      // ALNS params
      descr_file << "ALNS parameters: no " << k << " \n";
      descr_file << "\tIterations: " << iterations << "\n";
      descr_file << "\tSegments: " << segments << "\n";
      descr_file << "\tscore1\tscore2\tscore3\treaction factor\tcooling rate\tworsening factor\twait k It\tY\tw' shake = w * \tc' shake\n";
      descr_file << "\t28\t9\t3\t0.1\t0.999875\t0.005\t12500\t20\t0.5\t0.999\n";
      // Runtime
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
      descr_file << "Runtime: " << duration.count() << " milliseconds.\n";
      // in minutes
      descr_file << "Runtime: " << duration.count()/60000.0 << " minutes.\n";
      descr_file.close();

      std::string filename = "best_sol";
      pi.saveRecapToFile(filename, out_folder);
      filename = "final_batches";
      pi.saveBatches(true, filename, out_folder);
    }



    return 0;
  }

}
