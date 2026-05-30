import { dlopen, FFIType } from "bun:ffi";
import { spawnSync } from "bun"

type Runenv = Bun;

const lib = dlopen("./build/libcache.so", {
  cache_init: { args: [FFIType.cstring], returns: FFIType.i32 },
  cache_put: { args: [FFIType.cstring, FFIType.cstring], returns: FFIType.void },
  cache_get: { args: [FFIType.cstring], returns: FFIType.cstring },
  cache_destroy: { args: [], returns: FFIType.void }
});

spawnSync(["rm", "-rf", "/dev/shm/bench_cache"]);

const SHM_NAME = Buffer.from("/bench_cache\0");
if (lib.symbols.cache_init(SHM_NAME) !== 0) throw new Error("Cannot init cache");

const ITERATIONS = 1_000_000;
const SIZE_BATCH = 10000; 
const half = ITERATIONS / 2;

const keys = new Array(ITERATIONS);
const values = new Array(ITERATIONS);

for (let i = 0; i < ITERATIONS; i++) {
  keys[i] = Buffer.from(`key:${i}\0`);
  values[i] = Buffer.from(`value:${i}\0`);
}

const runCoroutine = async (corrutina: AsyncGenerator) => {
  for await (const _ of corrutina) {
  }
};

async function* writesGenerator(start: number, end: number, size: number) {
  for (let i = start; i < end; i++) {
    lib.symbols.cache_put(keys[i], values[i]);

    if (i % size === 0) {
      yield; 
    }
  }
}

async function benchmarkPut() {
  const start = Bun.nanoseconds();

  const writes1 = writesGenerator(0, half, SIZE_BATCH);
  const writes2 = writesGenerator(half, ITERATIONS, SIZE_BATCH);

  await Promise.all([
    runCoroutine(writes1),
    runCoroutine(writes2)
  ]);

  const end = Bun.nanoseconds();
  const seconds = Number(end - start) / 1e9;

  console.log("[BUN][PUT] benchmark (yield)");
  console.log(`[BUN][PUT] expended time: ${seconds.toFixed(4)} sec | Ops/sec: ${Math.floor(ITERATIONS / seconds)}\n`);
}

async function* readsGenerator(start: number, end: number, size: number) {
  for (let i = start; i < end; i++) {
    lib.symbols.cache_get(keys[i]);

    if (i % size === 0) {
      yield; 
    }
  }
}

async function benchmarkGet() {
  const start = Bun.nanoseconds();

  const read1 = readsGenerator(0, half, SIZE_BATCH);
  const read2 = readsGenerator(half, ITERATIONS, SIZE_BATCH);

  await Promise.all([
    runCoroutine(read1),
    runCoroutine(read2)
  ]);

  const end = Bun.nanoseconds();
  const seconds = Number(end - start) / 1e9;

  console.log("[BUN][GET] benchmark (yield)");
  console.log(`[BUN][GET] expended time: ${seconds.toFixed(4)} sec | Ops/sec: ${Math.floor(ITERATIONS / seconds)}`);
}

await benchmarkPut();
await benchmarkGet();

lib.symbols.cache_destroy();
