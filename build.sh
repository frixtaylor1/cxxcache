g++ -O3 -shared -fPIC ./src/**/*.cpp -o ./build/libcache.so -lrt -lpthread

bun build --compile --outfile=./build/test_bun ./examples/example_bun_stress.ts