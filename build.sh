g++ -O3 -shared -fPIC ./src/*.cpp -o ./build/libcache.so -lrt -lpthread

bun build --compile --outfile=./build/test_bun ./test_bun.ts