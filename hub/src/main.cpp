#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <unistd.h>

#include <pqxx/pqxx>

#include "config.h"
#include "device.h"
#include "gateway.h"
#include "hub.h"
#include "measurement.h"

#include "utility/hexify.h"
#include "utility/sync_queue.h"

using namespace xiaxr;
using namespace std::chrono_literals;

namespace {
std::condition_variable exit_cv;
std::mutex exit_cv_m;

volatile std::sig_atomic_t signal_status;

} // namespace

void check_root() {
  if (geteuid()) {
    std::cerr << "Xiaxr Hub must be run as root - Exiting" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void check_db() {
  try {
    auto x = pqxx::connection(tsdb_connection);
  } catch (...) {
    std::cerr << "Xiaxr Hub cannot connect to database - Exiting" << std::endl;
    exit(EXIT_FAILURE);
  }
}

void handle_signal(int signum) {
  if (signum == SIGINT || signum == SIGTERM) {
    signal_status = 1;
    exit_cv.notify_all();
  }
}

void print(std::string x) {
  static std::mutex mutex;
  std::unique_lock<std::mutex> locker(mutex);
  std::cout << x;
}

void process_measurement(const Configuration &config,
                         SyncQueue<Measurement> &q) {

  auto conn = pqxx::connection(tsdb_connection);

  conn.prepare("insert_measurement",
               "INSERT INTO measurements(time, device, measurement_id, tag, "
               "type, unit, value) VALUES ($1, $2, $3, $4, $5, $6, $7)");

  while (1) {
    auto msg = q.pop();
    if (msg.is_terminate()) {
      return;
    }

    {
      pqxx::work w{conn};
      w.exec_prepared("insert_measurement", msg.db_ts(), msg.db_device_id(),
                      msg.db_id(), msg.db_tag(), msg.db_type(), msg.db_unit(),
                      msg.db_value());
      w.commit();
    }

    print(msg.as_json());
  }
}

void rf24network_pump(const Configuration &config, SyncQueue<Measurement> &q) {
  auto gateway = Gateway(config);
  gateway.begin();

  while (1) {
    gateway.update([&]<typename T>(const T &msg) { q.push(msg); });
    {
      std::unique_lock<std::mutex> lk(exit_cv_m);
      if (exit_cv.wait_until(
              lk, std::chrono::system_clock::now() + network::network_delay,
              []() { return signal_status == 1; })) {
        q.push(Measurement::terminate_message());
        return;
      }
    }
  }
}

int main() {
  check_root();

  auto config = Configuration::load();

  check_db();

  std::signal(SIGINT, handle_signal);
  std::signal(SIGTERM, handle_signal);

  SyncQueue<Measurement> q;

  std::thread rf24_pump(
      std::bind(rf24network_pump, std::ref(config), std::ref(q)));

  std::thread process_pump(
      std::bind(process_measurement, std::ref(config), std::ref(q)));

  rf24_pump.join();
  process_pump.join();

  return EXIT_SUCCESS;
}