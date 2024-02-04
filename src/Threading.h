#pragma once

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

template <typename T>
class ThreadedQueue
{
 public:
  ThreadedQueue() = default;

  ~ThreadedQueue() { stop(); }

  void push(const T& item)
  {
    {
      std::unique_lock lock(m_mutex);
      m_queue.push_front(item);
    }
    m_condition.notify_one();
  }

  void push(T&& item)
  {
    {
      std::unique_lock lock(m_mutex);
      m_queue.push_front(std::move(item));
    }
    m_condition.notify_one();
  }

  bool pop(T& item)
  {
    std::unique_lock lock(m_mutex);
    m_condition.wait(lock, [&]() { return !m_queue.empty() || m_stop; });

    if (m_stop) {
      return false;
    }

    item = std::move(m_queue.front());
    m_queue.pop_front();
    return true;
  }

  void clear()
  {
    std::unique_lock lock(m_mutex);
    m_queue = {};
    m_condition.notify_all();
  }

  size_t size() const
  {
    std::unique_lock lock(m_mutex);
    return m_queue.size();
  }

  bool empty() const
  {
    std::unique_lock lock(m_mutex);
    return m_queue.empty();
  }

  void stop()
  {
    {
      std::unique_lock lock(m_mutex);
      m_stop = true;
    }
    m_condition.notify_all();
  }

 private:
  bool m_stop = false;
  std::deque<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_condition;
};

class ThreadPool
{
 public:
  using Work = std::function<void(void)>;

  ThreadPool(size_t thread_count = std::thread::hardware_concurrency())
  {
    m_threads.reserve(thread_count);

    for (auto i = 0U; i < thread_count; ++i) {
      auto worker = [this]() {
        while (true) {
          Work work{};

          bool success = m_queue.pop(work);

          if (!(work && success)) break;

          work();
        }
      };

      m_threads.push_back(std::thread(worker));
    }
  }

  ~ThreadPool()
  {
    for (auto i = 0U; i < m_threads.size(); ++i) {
      m_queue.push(Work{});  // null item
    }

    for (auto& t : m_threads) {
      t.join();
    }
  }

  void assign_work(Work work) { m_queue.push(std::move(work)); }

  void clear_queue() { m_queue.clear(); }

 private:
  ThreadedQueue<Work> m_queue;
  std::vector<std::thread> m_threads;
};
