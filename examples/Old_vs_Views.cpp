#include <cstddef>  // ptrdiff_t, size_t
#include <iterator> // forward_iterator_tag
#include <utility>  // exchange
#include <vector>   // vector
#include <ranges>   // subrange

constexpr const int n = 5; // Numbers of elements to get from the sequence
constexpr const int k = 3; // Divisible by

static void FibOldFashion(benchmark::State& state) {
  // Code inside this loop is measured repeatedly
  for (auto _ : state) {
    int found = 0;
    std::vector<std::size_t> values;
    std::size_t current = 1, next = 1;
    values.reserve(n);

    while( found < n){
      if(current % k == 0) {
        values.push_back(current);
        ++found;
      }
      current = std::exchange(next, current + next);
    }
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(values);
  }
}
// Register the function as a benchmark
BENCHMARK(FibOldFashion);



struct fibonacci_iterator {
  using value_type        = std::size_t;
  using difference_type   = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  const value_type &operator*() const { return current; }

  fibonacci_iterator &operator++()
  {
    current = std::exchange( next, current + next );
    return *this;
  }

  fibonacci_iterator operator++( int )
  {
    auto temp = *this;
    ++( *this );
    return temp;
  }

  bool operator==( const fibonacci_iterator & ) const = default;

private:
  value_type current = 1;
  value_type next    = 1;
};

constexpr auto fibonacci = std::ranges::subrange< fibonacci_iterator, std::unreachable_sentinel_t >{};

static void FibViewFashion(benchmark::State& state) {
  // Code before the loop is not measured
  for (auto _ : state) {
    std::vector<std::size_t> values;
    values.reserve(n);
    auto view = fibonacci |
                  std::views::filter([](auto item){ return item % k == 0;}) |
                  std::views::take(n);
    std::ranges::copy(view, std::begin(values)); // std::ranges::copy is 10% faster then the loop below on gcc 12.2 O3
    // for(const auto & item: values) {
    //   ret.push_back(item);
    // }
    // Make sure the variable is not optimized away by compiler
    benchmark::DoNotOptimize(values);
  }
}
BENCHMARK(FibViewFashion);