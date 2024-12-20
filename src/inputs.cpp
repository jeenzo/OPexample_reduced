#include "inputs.hpp"


std::vector<double> readWarehouseChar(const std::string& filename) {
  // Return {num_aisles, num_blocks, num_locs, loc_wx, loc_wy, w_aisle, w_cross_aisle, x0, y0}
  std::vector<double> params;
  std::cout << "Reading " << filename << "... ";
  std::ifstream file(filename);
  // Read second line of file, elements are separated by commas
  if (file.is_open()) {
      std::string line;
      std::getline(file, line);  // skip first line
      std::getline(file, line);
      std::istringstream iss(line);
      std::string value;
      while (std::getline(iss, value, ',')) {
          params.push_back(std::stod(value));
      }
      file.close();
  } else {
      std::cerr << "Unable to open file " << filename
                << " for reading." << std::endl;
  }
  std::cout << "Done\n";

  return params;
}

std::vector<std::vector<double>> readLocationMatrix(const std::string& filename) {
  std::vector<std::vector<double>> loc_mat;
  std::cout << "Reading " << filename << "... ";
  std::ifstream file(filename);

  // Elements are separated by commas
  if (file.is_open()) {
      std::string line;
      std::getline(file, line);  // skip line
      // Read first element of 2nd line 
      std::getline(file, line);
      int N = std::stoi(line.substr(line.find("\t")+1));
      std::getline(file, line);  // skip line
      std::getline(file, line);  // skip line (header)
      // Resize loc_mat: N rows, 5 cols
      loc_mat.resize(N+1, std::vector<double>(5));
      // Read remaining lines
      int i = 0;
      while (std::getline(file, line)) {
          std::istringstream iss(line);
          std::string value;
          std::getline(iss, value, ',');  // loc_id
          loc_mat[i][0] = std::stod(value);
          std::getline(iss, value, ',');  // x
          loc_mat[i][1] = std::stod(value);
          std::getline(iss, value, ',');  // y
          loc_mat[i][2] = std::stod(value);
          std::getline(iss, value, ',');  // block_nr
          loc_mat[i][3] = std::stod(value);
          std::getline(iss, value, ',');  // aisle_side
          loc_mat[i][4] = std::stod(value);
          i++;
      }
      file.close();
  } else {
      std::cerr << "Unable to open file " << filename
                << " for reading." << std::endl;
  }
  std::cout << "Done\n";

  return loc_mat;
}

std::vector<std::vector<int>> readOrdersFilePy(const std::string& filename) {
  std::vector<std::vector<int>> orders;
  std::cout << "Reading " << filename << "... ";
  // Input is a df with cols [order_id, prod_id, qty]
  std::ifstream file(filename);
  // Fist line contains number of orders, second line is df header
  if (file.is_open()) {
      // Save number of orders
      std::string line;
      std::getline(file, line);
      int M = std::stoi(line.substr(line.find("\t")+1));
      // Initialize orders vector
      orders.resize(M);
      // read orders
      std::getline(file, line);  // skip header line
      int order_id, prod_id, qty;
      while (std::getline(file, line)) {
          std::istringstream iss(line);
          std::string value;
          std::getline(iss, value, ',');  // order_id
          order_id = std::stod(value);
          std::getline(iss, value, ',');  // prod_id
          prod_id = std::stod(value);
          std::getline(iss, value, ',');  // qty
          qty = std::stod(value);
          // Append to orders[order_id-1] as many times as qty
          for (int i = 0; i < qty; i++) orders[order_id-1].push_back(prod_id);
      }
      file.close();
  } else {
      std::cerr << "Unable to open file " << filename
                << " for reading." << std::endl;
  }
  std::cout << "Done\n";

  return orders;
}

std::vector<std::vector<double>> readMatrix(const std::string& filename,
                                            const bool header) {
  std::vector<std::vector<double>> matrix;
  std::cout << "Reading " << filename << "... ";
  std::ifstream file(filename);
  // Elements are separated by commas
  if (file.is_open()) {
    std::string line;
    if (header) std::getline(file, line);
    while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::string value;
      std::vector<double> row;
      while (std::getline(iss, value, ',')) {
        row.push_back(std::stod(value));
      }
      matrix.push_back(row);
    }
    file.close();
  } else {
      std::cerr << "Unable to open file " << filename
                << " for reading." << std::endl;
  }
  std::cout << "Done\n";

  return matrix;
}

InputData readInputs(const std::string& wh_file,
                     const std::string& orders_file,
                     const std::string& pen_file,
                     const std::string& time_file,
                     const std::string& worker_file) {
  InputData data;
  // Read location matrix
  data.loc_mat = readLocationMatrix(wh_file);
  // Read wh params
  std::vector<double> wh_params = readWarehouseChar(wh_file);
  data.N = wh_params[0];
  data.n_aisles = wh_params[1];
  data.n_blocks = wh_params[2];
  data.n_loc_per_aisle = wh_params[3];
  data.wx = wh_params[4];
  data.wy = wh_params[5];
  data.w_aisle = wh_params[6];
  data.w_cross = wh_params[7];
  data.x0 = wh_params[8];
  data.y0 = wh_params[9];
  // Read orders
  data.order_lines = readOrdersFilePy(orders_file);
  // Read penalty matrix
  data.pen_mat = readMatrix(pen_file);
  // Read time matrix
  data.time_mat = readMatrix(time_file);
  // Read worker data
  data.worker_data = readMatrix(worker_file, true);

  return data;
}

bool inputValidator(InputData data, std::vector<int>& idx_workers) {
  // Check if idx_workers is not empty
  if (idx_workers.size() == 0) {
    std::cout << "idx_workers is empty.\n";
    return false;
  }
  // Get W from data.worker_data dimension
  int W = data.worker_data.size();
  // Check if all idx in idx_workers are in [0, W)
  for (int i = 0; i < idx_workers.size(); i++) {
    if (idx_workers[i] < 0 || idx_workers[i] >= W) {
      std::cout << "idx_workers contains an index out of range.\n";
      return false;
    }
  }
  // Check if number of columns of pen_mat, time_mat are W
  if (data.pen_mat[0].size() != W || data.time_mat[0].size() != W) {
    std::cout << "pen_mat or time_mat has a wrong number of columns.\n";
    return false;
  }
  // Get N as data.loc_mat dimension
  int N = data.loc_mat.size() - 1;
  // Check if all order_lines are in [1, N]
  for (int i = 0; i < data.order_lines.size(); i++) {
    for (int j = 0; j < data.order_lines[i].size(); j++) {
      if (data.order_lines[i][j] < 1 || data.order_lines[i][j] > N) {
        std::cout << "order_lines contains an index out of range.\n";
        return false;
      }
    }
  }
  // Check if pen_mat, time_mat have dimension N+1
  if (data.pen_mat.size() != N+1 || data.time_mat.size() != N+1) {
    std::cout << "pen_mat or time_mat has a wrong number of rows.\n";
    return false;
  }

  // std::cout << "Input data is valid.\n";
  return true;
}

std::vector<std::vector<std::vector<std::vector<int>>>> readBatches(const std::string& filename) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
        return {};
    }

    std::vector<std::vector<std::vector<std::vector<int>>>> result;
    std::string line;
    std::vector<std::vector<std::vector<int>>> worker;
    std::vector<std::vector<int>> batch;
    std::vector<int> order;

    while (std::getline(inputFile, line)) {
      if (line[0] == 'W') {
          if (!worker.empty()) {
              worker.push_back(batch);
              batch.clear();
              result.push_back(worker);
              worker.clear();
          } 
      } else if (line[0] == 'B') {
          if (!batch.empty()) {
              worker.push_back(batch);
              batch.clear();
          }
      } else {
          std::stringstream ss(line);
          int item;
          while (ss >> item) {
              order.push_back(item);
              if (ss.peek() == ',') {
                  ss.ignore();
              }
          }
          batch.push_back(order);
          order.clear();
      }
    }

    if (!batch.empty()) {
        worker.push_back(batch);
    }
    if (!worker.empty()) {
        result.push_back(worker);
    }

    inputFile.close();
    return result;
}
