#ifndef __CJAMCPP_HPP__
#define __CJAMCPP_HPP__

#include <cstdint>
#include <cstdio>
#include <cctype>
#include <vector>
#include <map>
#include <future>
#include <thread>
#include <functional>
#include <sstream>

namespace std
{
  template <typename T>
  int sgn(T val)
  {
    return (T(0) < val) - (val < T(0));
  }
}

struct worker
{
  worker()
    : tasks()
  {
  }

  worker(worker&& other)
    : tasks(std::move(other.tasks))
  {
  }

  void exec_in_new_thread()
  {
    std::thread([this](){
      for (auto i = tasks.begin(); i != tasks.end(); ++i)
        (*i)();
    }).detach();
  }

  std::vector<std::packaged_task<std::string()>> tasks;
};

template <typename TCase>
class runner
{
public:
  runner()
  {
  }

  void run()
  {
    setup_io();
    uint32_t T;
    scanf("%d\n", &T);
    std::vector<TCase> cases;
    cases.reserve(T);
    for (uint32_t i = 0; i < T; ++i)
    {
      cases.emplace_back();
      cases.back().read();
    }
    uint32_t num_workers = std::thread::hardware_concurrency() - 2;
    std::vector<std::future<std::string>> results;
    results.reserve(T);
    std::vector<worker> workers;
    workers.resize(num_workers);
    for (uint32_t i = 0; i < T; ++i)
    {
      std::reference_wrapper<TCase> case_ref = cases[i];
      auto task = std::packaged_task<std::string()>([case_ref]() {
        case_ref.get().solve();
        return case_ref.get().str();
      });
      results.emplace_back(task.get_future());
      workers[i % num_workers].tasks.emplace_back(std::move(task));
    }
    for (uint32_t i = 0; i < num_workers; ++i)
      workers[i].exec_in_new_thread();
    for (uint32_t i = 0; i < T; ++i)
    {
      printf("Case #%d: ", i + 1);
      fflush(stdout);
      results[i].wait();
      printf("%s\n", results[i].get().c_str());
    }
  }

private:
  void setup_io()
  {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
  }
};

#endif // __CJAMCPP_HPP__