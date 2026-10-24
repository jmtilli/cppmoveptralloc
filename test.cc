#include "moveptralloc.h"
#include <vector>

int main(int argc, char **argv)
{
  std::shared_ptr<MovePtrAllocatorImpl> impl(new MovePtrAllocatorImpl(128*1024, 32*1024));
  MovePtrTypeAllocator<double> dblalloc(impl);
  using Traits = std::allocator_traits<MovePtrTypeAllocator<double> >;
  std::vector<int, MovePtrTypeAllocator<int> > vec(impl);
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
  d = Traits::allocate(dblalloc, 1);
  Traits::construct(dblalloc, d, 1.5);
  Traits::destroy(dblalloc, d);
  Traits::deallocate(dblalloc, d, 1);

  d = Traits::allocate(dblalloc, 3);
  Traits::construct(dblalloc, &d[0], 2.5);
  Traits::construct(dblalloc, &d[1], 3.5);
  Traits::construct(dblalloc, &d[2], 4.5);
  Traits::destroy(dblalloc, &d[2]);
  Traits::destroy(dblalloc, &d[1]);
  Traits::destroy(dblalloc, &d[0]);
  Traits::deallocate(dblalloc, d, 3);

  return 0;
}
