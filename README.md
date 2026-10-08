*This project has been created as part of the 42 curriculum by wwiedijk.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads. A group of coders sit in a circle around a shared Quantum Compiler. Between every pair of neighbouring coders lies one USB dongle (there are exactly as many dongles as coders). To compile, a coder needs **both** the dongle on their left and the one on their right. After compiling they put the dongles back, then debug, then refactor, and try to compile again.

If a coder does not start a compile within `time_to_burnout` ms of the start of their previous compile (or the start of the simulation), they burn out and the simulation stops. The simulation also stops when every coder has compiled at least `number_of_compiles_required` times.

The goal is to share the dongles fairly and without deadlock so that nobody burns out, while respecting:

- a **cooldown** after a dongle is released, during which it cannot be taken again,
- a **scheduler** (`fifo` or `edf`) that decides who gets a contested dongle,
- a separate **monitor** thread that detects burnout within 10 ms,
- **serialized logging** so that lines never interleave.

### Source files

| File | Responsibility |
|---|---|
| `main.c` | Entry point, thread joining, cleanup |
| `parsing.c` / `errors.c` | Argument validation and error messages |
| `setup.c` | Allocation and initialisation of coders, dongles, mutexes and conditions |
| `coder.c` | Coder thread routine (compile → debug → refactor), time helper, logging |
| `dongle.c` | Dongle acquisition, ordering of dongles, readiness checks, single-coder case |
| `dongle_helper.c` | Scheduler key, registering/unregistering a coder in the dongle queues |
| `dongle_queue.c` | The wait loop that blocks a coder until both dongles can be taken |
| `heap.c` | Min-heap used as the waiting queue of each dongle |
| `release.c` | Dongle release with cooldown, thread-safe read of the stop flag |
| `monitor.c` | Monitor thread: burnout and completion detection, simulation shutdown |

## Instructions

### Compilation

```bash
make        # builds ./codexion with -Wall -Wextra -Werror -pthread
make clean  # removes object files
make fclean # removes objects and the binary
make re     # fclean + all
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | Number of coders (and dongles), ≥ 1 |
| `time_to_burnout` | ms a coder may go without starting a compile |
| `time_to_compile` | ms spent compiling (holding two dongles) |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | Compiles each coder needs for the simulation to end successfully, ≥ 1 |
| `dongle_cooldown` | ms a dongle stays unavailable after being released |
| `scheduler` | `fifo` or `edf` |

All arguments are mandatory. Non-integers, negative values and any scheduler other than `fifo` / `edf` are rejected with an error message on `stderr`.

### Example

```bash
./codexion 5 800 200 200 200 3 10 edf
```

Output format (`timestamp_in_ms coder_id message`):

```
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
400 1 is refactoring
...
```

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock needs all four Coffman conditions at once. The implementation breaks two of them and the other two are inherent to the problem:

- **Hold and wait** is broken. A coder never holds one dongle while waiting for the other. Both dongles are only marked as taken in a single critical section (under `table_mutex`), and only once *both* are available (cooldown passed) and the coder is at the head of both waiting queues.
- **Circular wait** is broken. The last coder (`id == number_of_coders`) requests their dongles in the opposite order (right, then left) compared to everyone else, so the request order around the table is not circular.
- **Mutual exclusion** and **no preemption** are inherent: a dongle can only be used by one coder, and is only given back voluntarily after compiling.

In addition, even-numbered coders delay their first attempt by `time_to_compile / 2`, which spreads the first requests over time and reduces initial contention.

### Single coder

With one coder there is only one dongle, so left and right are the same object. The coder takes it, logs `has taken a dongle`, and then waits until the simulation stops, since compiling is impossible with a single dongle. The monitor detects the burnout and stops the simulation.

### Starvation prevention and scheduling (heap)

Every dongle owns a small **binary min-heap** of waiters (`heap.c`), implemented with `sift_up` / `sift_down` on an array. At most two coders can ever queue on the same dongle (its two neighbours), so each heap has a capacity of 2.

A waiter is ordered by a **key**, then by coder id as a deterministic tie-break:

- `fifo`: key = the coder's `last_compile_ms`
- `edf`: key = `last_compile_ms + time_to_burnout` (the burnout deadline)

The root of the heap is the coder who is served first. A coder can only take the dongles when they are the root of **both** of their dongles' heaps, so a coder with an earlier key cannot be overtaken by a neighbour. When a coder is done waiting, they remove themselves from both heaps (`heap_remove`).

### Dongle cooldown

When a dongle is released, it is marked `available = true` with `available_at = now + dongle_cooldown`. A dongle is only considered ready when `available` is set **and** `now >= available_at`. A waiting coder computes the earliest upcoming `available_at` of the two dongles and sleeps with `pthread_cond_timedwait` until then, instead of busy-waiting. If no cooldown is pending, the coder blocks with `pthread_cond_wait` until a release or a stop wakes it up.

### Precise burnout detection

A dedicated monitor thread loops every 1 ms. For each coder that has not yet reached the required number of compiles, it checks `now >= last_compile_ms + time_to_burnout`. On detection it calls `exit_simulation`, which sets the stop flag and prints `burned out` immediately, well within the 10 ms requirement. The monitor also checks whether the total number of compiles has reached `number_of_compiles_required * number_of_coders`, which ends the simulation successfully.

### Clean shutdown

`exit_simulation` sets the stop flag, broadcasts on `table_cond` and on every coder's own condition variable, so any thread blocked while waiting for dongles wakes up, sees the stop flag and exits. `main` then joins all threads and frees all memory (coders, dongles and every dongle's heap array), and destroys all mutexes and condition variables.

### Log serialization

Every message goes through `log_state`, which holds `log_mutex` around a single `printf`. Lines can therefore never be interleaved. The `burned out` message is printed in the same critical section that sets the stop flag, and only if the flag was not already set, so it is printed at most once.

## Thread synchronization mechanisms

| Primitive | Protects / used for |
|---|---|
| one `pthread_t` per coder | Each coder runs `coder_routine` |
| `pthread_t monitor` | Burnout and completion detection |
| `log_mutex` (`pthread_mutex_t`) | Serializes all output to `stdout` |
| `stop_mutex` (`pthread_mutex_t`) | Protects the `stopped` flag (read via `is_stopped`, written in `exit_simulation`) |
| `table_mutex` (`pthread_mutex_t`) | Protects the shared dongle state: `available`, `available_at` and the waiting heaps |
| `table_cond` (`pthread_cond_t`) | Wakes coders waiting for dongles when a dongle is released or the simulation stops |
| per-coder `cond`, per-dongle `mutex` | Initialised in `setup`; used by `wake_all_coders` to wake any thread waiting on them during shutdown |

### Design choice: one table lock

All dongle state is protected by a single `table_mutex` instead of locking each dongle separately. A coder needs two dongles at once, and locking them one after another would reintroduce hold-and-wait and lock-ordering problems. With one lock, checking both dongles and taking both is a single atomic step.

### How race conditions are prevented

- **Dongle duplication:** a dongle can only be marked as taken inside `table_mutex`, after checking that it is `available`, past its cooldown, and that the coder is at the head of its heap. Two coders can never take the same dongle.
- **Atomic acquisition of both dongles:** the readiness check for both dongles and the update of both `available` flags happen under the same lock, so a coder never ends up with only one.
- **Heap consistency:** pushes and removals happen only while holding `table_mutex`.
- **Coder ↔ monitor communication:** the monitor never modifies dongle state. It only sets the stop flag (under `stop_mutex` and `table_mutex`) and wakes sleepers. Coders read the flag through `is_stopped()`, which takes `stop_mutex`, and are woken via `table_cond` and their own condition variables so they never stay blocked after the simulation ends.
- **Spurious wakeups:** every wait is inside a `while` loop that re-checks the full predicate (`!is_stopped && !dongles_ready`) after each wakeup.
- **Lost wakeups:** the predicate is checked while holding `table_mutex`, and releases and stops broadcast while holding it, so a wakeup cannot slip in between the check and the wait.

## Resources

### References

- POSIX Threads Programming – https://hpc-tutorials.llnl.gov/posix/
- `man 3 pthread_mutex_lock`, `man 3 pthread_cond_wait`, `man 3 pthread_cond_timedwait`, `man 2 gettimeofday`
- Dijkstra's Dining Philosophers problem and Coffman's conditions for deadlock
- Earliest Deadline First scheduling (Liu & Layland, 1973)
- Binary heaps (sift-up / sift-down) – any standard algorithms textbook

### Use of AI

AI was used for the following tasks:

- **README:** drafting and updating this README.
- **Brainstorming:** discussing the overall design (deadlock prevention, scheduling, how to structure the waiting queues).
- **Research:** looking up and understanding concepts such as Coffman's conditions, condition variables, `pthread_cond_timedwait`, EDF scheduling and binary heaps.
- **testing:** ai was used to test the code
- **to find errors and ideas to fix them :**
