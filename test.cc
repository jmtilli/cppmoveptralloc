#include "moveptralloc.h"
#include <vector>

namespace mpa = jmtilli::mpa;

int main(int argc, char **argv)
{
  // same as std::make_shared<mpa::MovePtrAllocatorImpl>(..., ...);
  auto impl = mpa::make_impl(512*1024, 128*1024);
  mpa::MovePtrTypeAllocator<double> dblalloc(impl);
  using DblTraits = std::allocator_traits<mpa::MovePtrTypeAllocator<double> >;
  std::vector<int, mpa::MovePtrTypeAllocator<int> > vec(impl);
  size_t i;
  double *d;

  vec.push_back(1);
  vec.push_back(2);
  vec.push_back(3);
  vec.clear();
  vec.push_back(4);
  vec.push_back(5);
  vec.push_back(6);
  vec.push_back(7);
  vec.push_back(8);
  vec.push_back(9);
  for (i = 0; i < 1000*1000; i++)
  {
    vec.push_back(i);
  }
  d = DblTraits::allocate(dblalloc, 1);
  DblTraits::construct(dblalloc, d, 1.5);
  DblTraits::destroy(dblalloc, d);
  DblTraits::deallocate(dblalloc, d, 1);

  d = DblTraits::allocate(dblalloc, 3);
  DblTraits::construct(dblalloc, &d[0], 2.5);
  DblTraits::construct(dblalloc, &d[1], 3.5);
  DblTraits::construct(dblalloc, &d[2], 4.5);
  DblTraits::destroy(dblalloc, &d[2]);
  DblTraits::destroy(dblalloc, &d[1]);
  DblTraits::destroy(dblalloc, &d[0]);
  DblTraits::deallocate(dblalloc, d, 3);

  return 0;
}
