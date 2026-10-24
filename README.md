# cppmoveptralloc

cppmoveptralloc is intended for allocation use cases in C++, where all
allocated objects are freed right when the program exits, or similarly if a
whole bunch of allocations should be freed at exactly the same time. The
allocator is a simple move-the-pointer allocator, which is the simplest and
fastest possible allocator ever possible. However, the drawback is that such an
allocator does not support freeing memory, except if all allocations are freed
at exactly the same moment and none are left. This is exactly what is done
here: the allocations are invalidated when the allocator goes out of scope.

The allocator is a header-only library, easy to be integrated to any project.
It is fully compatible with the standard C++ library. The test application
demonstrates its use.

The test application is compiled with stirmake. How to install it is described
in the web page of stirmake at https://github.com/Aalto5G/stirmake

When stirmake has been installed, type "smka" to compile.
