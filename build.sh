g++ -O3 -g -shared -fPIC ./src/**/*.cpp -o ./build/cxxcache.so -lrt -lpthread

bun build --compile --outfile=./build/test_bun ./examples/example_bun_stress.ts