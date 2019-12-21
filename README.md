# shims

Some old shims I invented, mostly for help during CTFs/debugging Linux binaries.

### memshim - provide a view into calls to \*alloc and free()
I've ran into plenty of times (especially during CTFs) where I needed to see memory allocations (from both the binary and loaded libraries) in real-time but debugging/strace/ltrace failed me somehow.  `memshim` simply hooks the \*alloc and free functions and gives you a view into what's happening.

Usage:
```
$ ./viewer.py

(in another terminal)

$ env LD_PRELOAD=./memshim64.so /path/to/binary

(and back in viewer.py....)

[=] Listening...
[=] Connection from PID 10292
fastbin_dup_into_stack -> malloc(8)                              = 0x1795260
fastbin_dup_into_stack -> malloc(8)                              = 0x1795280
fastbin_dup_into_stack -> malloc(8)                              = 0x17952a0
libc-2.27.so -> free(0x1795260)                          = <void>
libc-2.27.so -> free(0x1795280)                          = <void>
libc-2.27.so -> free(0x1795260)                          = <void>
fastbin_dup_into_stack -> malloc(8)                              = 0x1795260
fastbin_dup_into_stack -> malloc(8)                              = 0x1795280
fastbin_dup_into_stack -> malloc(8)                              = 0x1795260
fastbin_dup_into_stack -> malloc(8)                              = 0x7fffea917538

```

As we can see above, viewer will show 'what' made the call, the arguments, and the result.  Above is a fastbin dup into stack attack that results in an arbitrary malloc. Next steps would be to take `viewer` and have it automatically point out strange behavior, such as double-frees and/or use-after-frees.
