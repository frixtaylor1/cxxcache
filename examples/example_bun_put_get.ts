import { dlopen, FFIType } from "bun:ffi";
import { spawnSync } from "bun"

type Runenv = Bun;

const lib = dlopen("../build/libcache.so", {
  cache_init: { args: [FFIType.cstring], returns: FFIType.i32 },
  cache_put: { args: [FFIType.cstring, FFIType.cstring], returns: FFIType.void },
  cache_get: { args: [FFIType.cstring], returns: FFIType.cstring },
  cache_destroy: { args: [], returns: FFIType.void }
});

spawnSync(["rm", "-rf", "/dev/shm/bench_cache"]);

const SHM_NAME = Buffer.from("/bench_cache\0");
if (lib.symbols.cache_init(Buffer.from("/bench_cache\0")) !== 0) throw new Error("Cannot init cache");

lib.symbols.cache_put(Buffer.from("User:100\0"), Buffer.from(JSON.stringify({ status: true, time: (new Date()).toISOString(), name: "User100" })));

console.log(JSON.parse(lib.symbols.cache_get(Buffer.from("User:100\0"))));

lib.symbols.cache_destroy();