#include "moveptralloc.h"
#include <vector>

int main(int argc, char **argv)
{
  std::shared_ptr<MovePtrAllocatorImpl> impl(new MovePtrAllocatorImpl(128*1024, 32*1024));
  std::vector<int, MovePtrTypeAllocator<int> > vec(impl);
  size_t i;
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
  return 0;
}
