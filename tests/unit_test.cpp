#include <gtest/gtest.h>

#include <circular_buffer.hpp>

#include <numeric>
#include <vector>
#include <atomic>

std::uint64_t num_constructions = 0;
std::uint64_t num_deletions = 0;

struct leak_checker {
  leak_checker() { ++num_constructions; }

  ~leak_checker() { ++num_deletions; }

  leak_checker(const leak_checker&) { ++num_constructions; }

  leak_checker(leak_checker&&) noexcept { ++num_constructions; }

  leak_checker& operator=(const leak_checker&) = default;
  leak_checker& operator=(leak_checker&&) noexcept = default;

  std::vector<float> aa{ 1.f, 2.f, 4.f };
};

std::vector<int> gen_incremental_vector()
{
  constexpr auto   size = 512;
  std::vector<int> v(size);

  std::iota(v.begin(), v.end(), 0);

  return v;
}

const static auto inc_vec = gen_incremental_vector();

jm::static_circular_buffer<int, 16> gen_filled_cb(int size = 16)
{
  jm::static_circular_buffer<int, 16> cb;
  for (int i = 0; i < size; ++i)
    cb.push_back(i);

  return cb;
}

jm::dynamic_circular_buffer<int> dynamic_gen_filled_cb(int size = 16, int pushed_items = 16)
{
  jm::dynamic_circular_buffer<int> cb(size);
  for (int i = 0; i < pushed_items; ++i)
    cb.push_back(i);

  return cb;
}
TEST(leaks, static_buffer_leak_test) {
  {
    jm::static_circular_buffer<leak_checker, 2> buf;
    for (int i = 0; i < 128; ++i)
      buf.push_back({});
    jm::static_circular_buffer<leak_checker, 7> buf2(buf.begin(), buf.end());
    jm::static_circular_buffer<leak_checker, 2> buf3{ {}, {} };
    buf = buf3;
    buf2.clear();
  }
  EXPECT_EQ(num_constructions, num_deletions);
}
TEST(leaks, dynamic_buffer_leak_test) {
  {
    jm::dynamic_circular_buffer<leak_checker> buf;
    buf.reserve(2);
    for (int i = 0; i < 128; ++i)
      buf.push_back({});
    jm::dynamic_circular_buffer<leak_checker> buf2(buf.begin(), buf.end());
    jm::dynamic_circular_buffer<leak_checker> buf3{ {}, {} };
    buf = buf3;
    buf2.clear();
  }
  EXPECT_EQ(num_constructions, num_deletions);
}
TEST(conctruction, static_default_construction) {
  // const
  {
    const jm::static_circular_buffer<int, 16> cb;
    EXPECT_EQ(cb.size(), 0);

    EXPECT_EQ(cb.max_size(), 16);

    EXPECT_EQ(cb.begin(), cb.end());

    EXPECT_EQ(cb.cbegin(), cb.cend());

    EXPECT_EQ(cb.rbegin(), cb.rend());

    EXPECT_EQ(cb.crbegin(), cb.crend());
  }
  // non const
  {
    jm::static_circular_buffer<int, 16> cb;
    EXPECT_EQ(cb.size(), 0);

    EXPECT_EQ(cb.max_size(), 16);

    EXPECT_EQ(cb.begin(), cb.end());

    EXPECT_EQ(cb.cbegin(), cb.cend());

    EXPECT_EQ(cb.rbegin(), cb.rend());

    EXPECT_EQ(cb.crbegin(), cb.crend());
  }
}
TEST(conctruction, dynamic_default_construction) {
  // const
  {
    const jm::dynamic_circular_buffer<int> cb;
    EXPECT_EQ(cb.size(), 0);

    EXPECT_EQ(cb.max_size(), 0);

    EXPECT_EQ(cb.begin(), cb.end());

    EXPECT_EQ(cb.cbegin(), cb.cend());

    EXPECT_EQ(cb.rbegin(), cb.rend());

    EXPECT_EQ(cb.crbegin(), cb.crend());

  }
  // non const
  {
    jm::dynamic_circular_buffer<int> cb;

    EXPECT_EQ(cb.size(), 0);

    EXPECT_EQ(cb.max_size(), 0);

    EXPECT_EQ(cb.begin(), cb.end());

    EXPECT_EQ(cb.cbegin(), cb.cend());

    EXPECT_EQ(cb.rbegin(), cb.rend());

    EXPECT_EQ(cb.crbegin(), cb.crend());

    cb.reserve(5);
    EXPECT_EQ(cb.size(), 0);
    EXPECT_EQ(cb.max_size(), 5);

    cb.resize(10);
    EXPECT_EQ(cb.size(), 0);
    EXPECT_EQ(cb.max_size(), 10);

    cb.reserve(3);
    EXPECT_EQ(cb.size(), 0);
    EXPECT_EQ(cb.max_size(), 3);

    cb.resize(7);
    EXPECT_EQ(cb.size(), 0);
    EXPECT_EQ(cb.max_size(), 7);
  }
}
TEST(conctruction, static_copy_construction) {
  auto cb = gen_filled_cb(15);
  auto other = cb;

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}
TEST(conctruction, dynamic_copy_construction) {
  auto cb = dynamic_gen_filled_cb(15);
  auto other = cb;

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}
TEST(conctruction, static_move_construction) {
  auto cb = gen_filled_cb(15);
  auto temp = cb;
  auto other = std::move(temp);

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}
TEST(conctruction, dynamic_move_construction) {
  auto cb = dynamic_gen_filled_cb(15);
  auto temp = cb;
  auto other = std::move(temp);

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}
TEST(conctruction, static_initializer_list) {

  EXPECT_ANY_THROW((jm::static_circular_buffer<int, 4>{ 1, 2, 3, 5, 6 }));

  jm::static_circular_buffer<int, 4> buf{ { 1, 2, 3, 5 } };
}

TEST(iterators, static_construction) {
  {
    auto cb = gen_filled_cb(15);
    EXPECT_ANY_THROW((jm::static_circular_buffer<int, 4>{ cb.begin(), cb.end() }));

    jm::static_circular_buffer<int, 16> cb2(cb.begin(), cb.end());

    EXPECT_EQ(std::equal(cb.begin(), cb.end(), cb2.begin()), true);
    EXPECT_EQ(cb.size(), cb2.size());
  }

  jm::static_circular_buffer<int, 4> buf1{ 1, 2, 3, 4 };
  jm::static_circular_buffer<int, 4> buf2{ buf1.begin(), buf1.end() };

  EXPECT_EQ(std::equal(buf1.begin(), buf1.end(), buf2.begin()), true);
  EXPECT_EQ(buf1.size(), buf2.size());
}

TEST(iterators, dynamic_construction) {
  {
    auto cb = dynamic_gen_filled_cb(15);

    jm::dynamic_circular_buffer<int> cb2(cb.begin(), cb.end());

    EXPECT_EQ(std::equal(cb.begin(), cb.end(), cb2.begin()), true);
    EXPECT_EQ(cb.size(), cb2.size());
  }

  jm::dynamic_circular_buffer<int> buf1{ 1, 2, 3, 4 };
  jm::dynamic_circular_buffer<int> buf2{ buf1.begin(), buf1.end() };

  EXPECT_EQ(std::equal(buf1.begin(), buf1.end(), buf2.begin()), true);
  EXPECT_EQ(buf1.size(), buf2.size());
}
TEST(copy, static_copy) {
  auto cb = gen_filled_cb(15);

  decltype(cb) other;
  other = cb;

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}

TEST(copy, dynamic_copy) {
  auto cb = dynamic_gen_filled_cb(15);

  decltype(cb) other(cb.size());
  other = cb;

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}

TEST(move, static_assignment) {
  auto cb = gen_filled_cb(15);

  auto temp(cb);

  decltype(cb) other;
  other = std::move(temp);

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}

TEST(move, dynamic_assignment) {
  auto cb = dynamic_gen_filled_cb(15);

  auto temp(cb);

  decltype(cb) other;
  other = std::move(temp);

  EXPECT_EQ(std::equal(cb.begin(), cb.end(), other.begin()), true);
}

TEST(items, static_n_items_construction) {
  constexpr float               float_val = 2.f;
  jm::static_circular_buffer<float, 5> cb(4, float_val);
  for (auto item : cb)
    EXPECT_EQ(item, float_val);

  EXPECT_EQ(cb.size(), 4);
}

TEST(items, dynamic_n_items_construction) {
  constexpr float               float_val = 2.f;
  jm::dynamic_circular_buffer<float> cb(4, float_val);
  for (auto item : cb)
    EXPECT_EQ(item, float_val);

  EXPECT_EQ(cb.size(), 4);
}

TEST(buffer_capacity, reserve_items) {
  jm::dynamic_circular_buffer<float> cb;
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.max_size(), 0);
  EXPECT_EQ(cb.capacity(), 0);
  cb.reserve(3);
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.max_size(), 3);
  EXPECT_EQ(cb.capacity(), 3);

  {
    auto db = dynamic_gen_filled_cb(12, 6);
    EXPECT_EQ(db.size(), 6);
    db.resize(3);
    EXPECT_EQ(db.size(), 3);
    db.resize(20);
    EXPECT_EQ(db.size(), 3);
  }
  {
    jm::dynamic_circular_buffer<int> db(inc_vec.begin(), inc_vec.end());
    EXPECT_EQ(db.size(), inc_vec.size());

    db.resize(5);
    EXPECT_EQ(std::equal(db.begin(), db.end(), inc_vec.begin()), true);
    db.push_back(5);
    EXPECT_EQ(std::equal(db.begin(), db.end(), inc_vec.begin() + 1), true);
  }

}

TEST(buffer_capacity, resize_items) {
  jm::dynamic_circular_buffer<float> cb;
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.max_size(), 0);
  EXPECT_EQ(cb.capacity(), 0);
  cb.resize(3);
  EXPECT_EQ(cb.size(), 0);
  EXPECT_EQ(cb.max_size(), 3);
  EXPECT_EQ(cb.capacity(), 3);
}

TEST(buffer_capacity, static_clear_empty_full) {
  {
    auto cb = gen_filled_cb(12);
    EXPECT_EQ(cb.size(), 12);
    EXPECT_EQ(!cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    cb.clear();
    EXPECT_EQ(cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    EXPECT_EQ(cb.size(), 0);
  }

  {
    auto cb = gen_filled_cb(16);
    EXPECT_EQ(cb.size(), 16);
    EXPECT_EQ(cb.size(), cb.max_size());

    EXPECT_EQ(!cb.empty(), true);
    EXPECT_EQ(cb.full(), true);
    cb.clear();
    EXPECT_EQ(cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    EXPECT_EQ(cb.size(), 0);
  }
}

TEST(buffer_capacity, dynamic_clear_empty_full) {
  {
    auto cb = dynamic_gen_filled_cb(16, 12);
    EXPECT_EQ(cb.size(), 12);
    EXPECT_EQ(!cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    cb.clear();
    EXPECT_EQ(cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    EXPECT_EQ(cb.size(), 0);
  }

  {
    auto cb = dynamic_gen_filled_cb(16);
    EXPECT_EQ(cb.size(), 16);
    EXPECT_EQ(cb.size(), cb.max_size());

    EXPECT_EQ(!cb.empty(), true);
    EXPECT_EQ(cb.full(), true);
    cb.clear();
    EXPECT_EQ(cb.empty(), true);
    EXPECT_EQ(!cb.full(), true);
    EXPECT_EQ(cb.size(), 0);
  }
}

TEST(buffer_capacity, static_max_size) {
  jm::static_circular_buffer<int, 5> cb1;
  EXPECT_EQ(cb1.max_size(), 5);
}

TEST(buffer_capacity, dynamic_max_size) {
  jm::dynamic_circular_buffer<int> cb1(5);
  EXPECT_EQ(cb1.max_size(), 5);
}
TEST(buffer_capacity, dynamic_clear_shrink) {
  jm::dynamic_circular_buffer<int> cb1(5);
  EXPECT_EQ(cb1.max_size(), 5);
  cb1.clear();
  EXPECT_EQ(cb1.size(), 0);
  EXPECT_EQ(cb1.max_size(), 5);
}
TEST(push_pop, static_pop_back) {
  auto cb = gen_filled_cb();

  for (int i = 15; i > 0; --i) {
    EXPECT_EQ(cb.back(), i);
    cb.pop_back();
    EXPECT_EQ(cb.size(), i);
  }

  EXPECT_EQ(cb.front(), cb.back());
  cb.pop_back();
  cb.push_back(5);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 5);

  cb.push_back(6);
  EXPECT_EQ(cb.back(), 6);
  EXPECT_EQ(cb.front(), 5);
}

TEST(push_pop, dynamic_pop_back) {
  auto cb = dynamic_gen_filled_cb();

  for (int i = 15; i > 0; --i) {
    EXPECT_EQ(cb.back(), i);
    cb.pop_back();
    EXPECT_EQ(cb.size(), i);
  }

  EXPECT_EQ(cb.front(), cb.back());
  cb.pop_back();
  cb.push_back(5);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 5);

  cb.push_back(6);
  EXPECT_EQ(cb.back(), 6);
  EXPECT_EQ(cb.front(), 5);
}

TEST(push_pop, static_pop_front) {
  auto cb = gen_filled_cb();
  EXPECT_EQ(cb.front(), 0);

  for (int i = 0; i < 15; ++i) {
    EXPECT_EQ(cb.front(), i);
    cb.pop_front();
    EXPECT_EQ(cb.size(), 15 - i);
  }

  EXPECT_EQ(cb.front(), cb.back());
  cb.pop_front();
  cb.push_front(5);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 5);

  cb.push_front(6);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 6);
}

TEST(push_pop, dynamic_pop_front) {
  auto cb = dynamic_gen_filled_cb();
  EXPECT_EQ(cb.front(), 0);

  for (int i = 0; i < 15; ++i) {
    EXPECT_EQ(cb.front(), i);
    cb.pop_front();
    EXPECT_EQ(cb.size(), 15 - i);
  }

  EXPECT_EQ(cb.front(), cb.back());
  cb.pop_front();
  cb.push_front(5);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 5);

  cb.push_front(6);
  EXPECT_EQ(cb.back(), 5);
  EXPECT_EQ(cb.front(), 6);
}

TEST(push_pop, static_push_back) {
  jm::static_circular_buffer<int, 16> cb;

  for (auto i : inc_vec) {
    cb.push_back(i);
    EXPECT_EQ(cb.back(), i);
    EXPECT_EQ(*--cb.end(), i);
    auto front = cb.front();
    for (auto v : cb)
      EXPECT_EQ(v, front++);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, dynamic_push_back) {
  jm::dynamic_circular_buffer<int> cb(16);

  for (auto i : inc_vec) {
    cb.push_back(i);
    EXPECT_EQ(cb.back(), i);
    EXPECT_EQ(*--cb.end(), i);
    auto front = cb.front();
    for (auto v : cb)
      EXPECT_EQ(v, front++);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, static_push_front) {
  jm::static_circular_buffer<int, 16> cb;

  for (auto i : inc_vec) {
    cb.push_front(i);
    EXPECT_EQ(cb.front(), i);
    EXPECT_EQ(*cb.begin(), i);
    for (auto v : cb)
      EXPECT_EQ(v, i--);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, dynamic_push_front) {
  jm::dynamic_circular_buffer<int> cb(16);

  for (auto i : inc_vec) {
    cb.push_front(i);
    EXPECT_EQ(cb.front(), i);
    EXPECT_EQ(*cb.begin(), i);
    for (auto v : cb)
      EXPECT_EQ(v, i--);
  }
  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, static_emplace_back) {
  jm::static_circular_buffer<int, 16> cb;

  for (auto i : inc_vec) {
    cb.emplace_back(i);
    EXPECT_EQ(cb.back(), i);
    EXPECT_EQ(*--cb.end(), i);
    auto front = cb.front();
    for (auto v : cb)
      EXPECT_EQ(v, front++);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, dynamic_emplace_back) {
  jm::dynamic_circular_buffer<int> cb(16);

  for (auto i : inc_vec) {
    cb.emplace_back(i);
    EXPECT_EQ(cb.back(), i);
    EXPECT_EQ(*--cb.end(), i);
    auto front = cb.front();
    for (auto v : cb)
      EXPECT_EQ(v, front++);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, static_emplace_front) {
  jm::static_circular_buffer<int, 16> cb;

  for (auto i : inc_vec) {
    cb.emplace_front(i);
    EXPECT_EQ(cb.front(), i);
    EXPECT_EQ(*cb.begin(), i);
    for (auto v : cb)
      EXPECT_EQ(v, i--);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(push_pop, dynamic_emplace_front) {
  jm::dynamic_circular_buffer<int> cb(16);

  for (auto i : inc_vec) {
    cb.emplace_front(i);
    EXPECT_EQ(cb.front(), i);
    EXPECT_EQ(*cb.begin(), i);
    for (auto v : cb)
      EXPECT_EQ(v, i--);
  }

  EXPECT_EQ(cb.size(), cb.max_size());
}

TEST(iterators, static_cb_iterator_complies_stl) {
  using cbt = jm::static_circular_buffer<int, 4>;
  cbt cb;
  cb.push_back(1);
  cb.push_back(2);

  auto r = cb.begin();
  // CopyConstructible
  {
    // MoveConstructible
    {
      auto v = cb.begin();
      EXPECT_EQ(v, cb.begin());
      EXPECT_EQ(cbt::iterator(cb.begin()), cb.begin());
    }

    auto v = cb.begin();
    {
      auto u = r;
      EXPECT_EQ(u, v);
      EXPECT_EQ(v, r);
    }
    {
      EXPECT_EQ(cbt::iterator(v), v);
      EXPECT_EQ(v, r);
    }
  }

  // CopyAssignable
  { // MoveAssignable
    { auto t = cb.begin();
    t = cb.begin();
    static_assert(std::is_same<decltype(t = cb.begin()), cbt::iterator&>::value,
      "t = cb.begin() doesnt return T&");

    EXPECT_EQ((t = cb.end()), t);
    }
    {
      auto t = cb.begin();
      auto v = cb.end();
      static_assert(std::is_same<decltype(t = v), cbt::iterator&>::value,
        "t = cb.begin() doesnt return T&");

      EXPECT_EQ((t = v), t);
    }
  }

  // Swappable
  {
    using std::swap;
    auto u = cb.begin();
    auto t = cb.end();

    swap(u, t);
    EXPECT_EQ(u, cb.end());
    EXPECT_EQ(t, cb.begin());

    swap(t, u);
    EXPECT_EQ(u, cb.begin());
    EXPECT_EQ(t, cb.end());
  }

  {
    using value_type = cbt::iterator::value_type; // has all the typedefs
    using difference_type = cbt::iterator::difference_type;
    using reference = cbt::iterator::reference;
    using pointer = cbt::iterator::pointer;
    using iterator_category = cbt::iterator::iterator_category;
  }
  *r; // r is dereferenceable
  ++r; // r is incrementable
  static_assert(std::is_same<decltype(++r), decltype(r)&>::value,
    "++it doesnt return It&");
}

TEST(iterators, dynamic_cb_iterator_complies_stl) {
  using cbt = jm::dynamic_circular_buffer<int>;
  cbt cb(4);
  cb.push_back(1);
  cb.push_back(2);

  auto r = cb.begin();
  // CopyConstructible
  {
    // MoveConstructible
    {
      auto v = cb.begin();
      EXPECT_EQ(v, cb.begin());
      EXPECT_EQ(cbt::iterator(cb.begin()), cb.begin());
    }

    auto v = cb.begin();
    {
      auto u = r;
      EXPECT_EQ(u, v);
      EXPECT_EQ(v, r);
    }
    {
      EXPECT_EQ(cbt::iterator(v), v);
      EXPECT_EQ(v, r);
    }
  }

  // CopyAssignable
  { // MoveAssignable
    { auto t = cb.begin();
    t = cb.begin();
    static_assert(std::is_same<decltype(t = cb.begin()), cbt::iterator&>::value,
      "t = cb.begin() doesnt return T&");

    EXPECT_EQ((t = cb.end()), t);
    }
    {
      auto t = cb.begin();
      auto v = cb.end();
      static_assert(std::is_same<decltype(t = v), cbt::iterator&>::value,
        "t = cb.begin() doesnt return T&");

      EXPECT_EQ((t = v), t);
    }
  }

  // Swappable
  {
    using std::swap;
    auto u = cb.begin();
    auto t = cb.end();

    swap(u, t);
    EXPECT_EQ(u, cb.end());
    EXPECT_EQ(t, cb.begin());

    swap(t, u);
    EXPECT_EQ(u, cb.begin());
    EXPECT_EQ(t, cb.end());
  }

  {
    using value_type = cbt::iterator::value_type; // has all the typedefs
    using difference_type = cbt::iterator::difference_type;
    using reference = cbt::iterator::reference;
    using pointer = cbt::iterator::pointer;
    using iterator_category = cbt::iterator::iterator_category;
  }
  *r; // r is dereferenceable
  ++r; // r is incrementable
  static_assert(std::is_same<decltype(++r), decltype(r)&>::value,
    "++it doesnt return It&");
}
TEST(iterators, static_cb_iterator_complies_InputIterator) {
  using cbt = jm::static_circular_buffer<int, 4>;
  cbt cb;
  cb.push_back({ 1 });
  cb.push_back({ 2 });

  // EqualityComparable
  {
    auto a = cb.begin();
    auto b = cb.begin();
    auto c = b;

    EXPECT_EQ(a, a);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, a);
    EXPECT_EQ(a, c);

    static_assert(std::is_same<bool, decltype(a == b)>::value, "a==b not bool");
  }

  auto i = cb.begin();
  auto j = cb.end();

  EXPECT_EQ(((void)*i, *i), *i);

  EXPECT_EQ((i != j), (!(i == j)));

  cbt::iterator       it = cb.begin();
  cbt::const_iterator non_c_tttt(it);
  cbt::const_iterator non_c_it = it;
  non_c_it = it;
}

TEST(iterators, dynamic_cb_iterator_complies_InputIterator) {
  using cbt = jm::dynamic_circular_buffer<int>;
  cbt cb(4);
  cb.push_back({ 1 });
  cb.push_back({ 2 });

  // EqualityComparable
  {
    auto a = cb.begin();
    auto b = cb.begin();
    auto c = b;

    EXPECT_EQ(a, a);
    EXPECT_EQ(a, b);
    EXPECT_EQ(b, a);
    EXPECT_EQ(a, c);

    static_assert(std::is_same<bool, decltype(a == b)>::value, "a==b not bool");
  }

  auto i = cb.begin();
  auto j = cb.end();

  EXPECT_EQ(((void)*i, *i), *i);

  EXPECT_EQ((i != j), (!(i == j)));

  cbt::iterator       it = cb.begin();
  cbt::const_iterator non_c_tttt(it);
  cbt::const_iterator non_c_it = it;
  non_c_it = it;
}


#include <Eigen/Geometry>
#include <Eigen/StdVector>


TEST(eigen, custom_alloc)
{
   
    jm::dynamic_circular_buffer<Eigen::Vector3f, Eigen::aligned_allocator< Eigen::Vector3f >> buf1(8);
    jm::dynamic_circular_buffer<Eigen::Vector3f, Eigen::aligned_allocator< Eigen::Vector3f >> buf2(8);

    for (int i = 0; i < 128; ++i) {
      buf1.push_back(Eigen::Vector3f::Random());
    }

    for (auto value : buf1) {
      buf2.push_back(value);
    }

    EXPECT_EQ(buf1.size(), buf2.size());
    EXPECT_EQ(std::equal(buf1.begin(), buf1.end(), buf2.begin()), true);
    *buf2.begin() = Eigen::Vector3f::Random();
    EXPECT_EQ(!std::equal(buf1.begin(), buf1.end(), buf2.begin()), true);
}