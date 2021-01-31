
#include <mutex>
#include <atomic>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include "spi.h"

pthread_mutex_t spiMutex = PTHREAD_MUTEX_INITIALIZER;

namespace {
std::atomic_flag bcmIsInitialized;
std::mutex spi_mutex;
}

void SPI::begin(int busNo, uint32_t spi_speed) {
  if (!bcmIsInitialized.test_and_set()) {
    if (!bcm2835_init()) {
      return;
    }
  }
  bcm2835_spi_begin();
}

void SPI::beginTransaction(SPISettings settings) {
  pthread_mutex_lock(&spiMutex);
  setBitOrder(settings.border);
  setDataMode(settings.dmode);
  setClockDivider(settings.clck);
}

void SPI::endTransaction() { pthread_mutex_unlock(&spiMutex); }
