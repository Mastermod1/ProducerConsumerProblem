#include <bits/stdc++.h>

#ifdef DBG
constexpr bool g_debug = true;
#else
constexpr bool g_debug = false;
#endif

constexpr std::size_t g_data_size = 10'000;
constexpr std::size_t g_storage_size = 20;

using storage_t = std::array<int, g_storage_size>;

std::size_t g_current_storage_size = 0;
std::condition_variable g_cv;
std::mutex g_mtx;

auto random_time()
{
    return (rand()%6)+1;
}

void producer(storage_t& storage, int id)
{
    int pos = 0;
    std::size_t produced_count = 0;
    while(produced_count < g_data_size)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(random_time()));
        std::unique_lock lck(g_mtx);
        if (g_current_storage_size >= g_storage_size)
            g_cv.wait(lck, []{ return g_current_storage_size < g_storage_size; });
        storage[pos] = 10;
        g_current_storage_size++;
        produced_count++;
        if constexpr (g_debug)
            std::cout << "Produced[" << id << "]: " << storage[pos] << "\n";
        g_cv.notify_one();
        pos = (pos + 1) % g_storage_size;
    }
}

void consumer(storage_t& storage, int id)
{
    int pos = 0;
    std::size_t consumed_count = 0;
    while(consumed_count < g_data_size)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(random_time()));
        std::unique_lock lck(g_mtx);
        if (g_current_storage_size == 0)
            g_cv.wait(lck, []{ return g_current_storage_size != 0; });
        int consumed = storage[pos];
        g_current_storage_size--;
        consumed_count++;
        assert(consumed != -1);
        storage[pos] = -1;
        g_cv.notify_one();
        if constexpr (g_debug)
            std::cout << "Consumed[" << id << "]: " << consumed << "\n";
        pos = (pos + 1) % g_storage_size;
    }
}

int main()
{
    srand(time(0)); 
    storage_t storage;
    storage.fill(-1);
    std::thread producer_thread(producer, std::ref(storage), 1);
    std::thread consumer_thread(consumer, std::ref(storage), 1);
    producer_thread.join();
    consumer_thread.join();
    std::cout << "Finished\n";
}
