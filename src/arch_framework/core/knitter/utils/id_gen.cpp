#include "knitter/utils/id_gen.h"
#include <atomic>
#include <ctime>
#include <random>
#include "knitter/utils/string_printf.h"

namespace knitter {
namespace utils {

uint64_t increased_unique_id() {
    static std::atomic<uint64_t> id_gen{0};
    id_gen.fetch_add(1, std::memory_order_relaxed);
    return id_gen.load();
}

std::string ts_unique_id() {
    time_t now = time(NULL);
    struct tm ltime;
    localtime_r(&now, &ltime);
    return knitter::utils::string_printf("%02d%02d%02d%02d%02d-%lu", ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour,
                                         ltime.tm_min, ltime.tm_sec, increased_unique_id());
}

std::string ts_unique_client_id() {
    time_t now = time(NULL);
    struct tm ltime;
    localtime_r(&now, &ltime);
    static std::atomic<uint64_t> id_gen{0};
    id_gen.fetch_add(1, std::memory_order_relaxed);
    return knitter::utils::string_printf("%02d%02d%02d%02d%02d-%lu", ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour,
                                         ltime.tm_min, ltime.tm_sec, id_gen.load());
}

std::string ts_unique_server_id() {
    time_t now = time(NULL);
    struct tm ltime;
    localtime_r(&now, &ltime);
    static std::atomic<uint64_t> id_gen{0};
    id_gen.fetch_add(1, std::memory_order_relaxed);
    return knitter::utils::string_printf("%02d%02d%02d%02d%02d-%lu", ltime.tm_mon + 1, ltime.tm_mday, ltime.tm_hour,
                                         ltime.tm_min, ltime.tm_sec, id_gen.load());
}

uint64_t random_id() {
    thread_local std::random_device rd;
    thread_local std::mt19937 gen(rd());
    return gen();
}

uint64_t random_id_greater(uint64_t min) {
    int try_times = 0;
    uint64_t candidate = min + 1;
    for (int i = 0; i < try_times; i++) {
        candidate = random_id();
        if (candidate > min) {
            return candidate;
        }
    }
    return candidate + min;
}

uint64_t random_id_less(uint64_t max) {
    return random_id() % max;
}

uint64_t random_id_between(uint64_t min, uint64_t max) {
    if (min > max) {
        return 0;
    }
    if (min == max) {
        return min;
    }
    return min + random_id() % (max - min);
}

std::pair<std::string, uint64_t> ts_unique_pair() {
    time_t now = time(NULL);
    struct tm ltime;
    localtime_r(&now, &ltime);

    auto id = increased_unique_id();
    return std::make_pair(knitter::utils::string_printf("%02d%02d%02d%02d%02d%lu", ltime.tm_mon + 1, ltime.tm_mday,
                                                        ltime.tm_hour, ltime.tm_min, ltime.tm_sec, id),
                          id);
}

}  // namespace utils
}  // namespace knitter
