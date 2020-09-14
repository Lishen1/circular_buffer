project( tests )

# Ensure we have pthreads
find_package(Threads REQUIRED)
enable_testing()
add_test_and_benchmark()


#set target executable
add_executable ( circular_buffer_tests      unit_test.cpp )
add_executable ( circular_buffer_benchmarks benchmark.cpp )


#add the library

target_link_libraries (circular_buffer_tests PRIVATE
 
	circular_buffer 
	Threads::Threads 
	eigen 
	gtest 
	gtest_main 
)


target_link_libraries (circular_buffer_benchmarks PRIVATE 
	circular_buffer 
	benchmark 
	Threads::Threads 
	eigen
)


add_test( circular_buffer_tests circular_buffer_tests )