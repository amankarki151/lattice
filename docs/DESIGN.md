# Design decisions

Notes on why things are built the way they are, including a few things
that were considered and dropped. Written mostly for my own benefit -
future me will not remember why any of this was done.

## Storage

### Write-ahead log before in-memory update

Every insert hits the log before the in-memory store. If the process
dies between the two, the record is on disk and comes back on the next
open. The other order loses it entirely - it would only ever have
existed in RAM.

The cost is a flush per write, which is slow. Batching is the obvious
optimization but it's not in yet, because a batching bug and a format
bug at the same time would be much harder to isolate than either alone.

### Separate segment files instead of only a WAL

A log that's never compacted only grows, and reading it back means
replaying every write ever made - including ones later overwritten by
a newer value for the same id. Segments hold the settled state, so
reading them means reading current data once, with no replay.

### Segment writes go to .tmp and get renamed

Rename on the same filesystem is atomic. From outside, the file either
has its old contents or the complete new ones. Writing directly into
the target means a crash partway through leaves a corrupt file sitting
where a valid one used to be, with no way for the next open to tell.

### Segments are mmap'd, not read()

The OS pages data in on demand, so opening a large segment is cheap and
only the parts actually touched cost anything.

The catch: record offsets aren't guaranteed to be aligned, since each
record's size depends on its own dimension. So fields get pulled out
with memcpy rather than by casting the mapped pointer to uint64_t* and
dereferencing it. A misaligned load through a cast is undefined
behaviour even on hardware that tolerates it in practice.

### Magic number and version in the segment header

Tested by deliberately corrupting a real segment file - overwriting the
first four bytes - and confirming the reader refused it rather than
reading the garbage as a valid header. Without the check, a corrupt
file gets interpreted as a real version and record count, which could
mean anything from wrong results to trying to allocate on a garbage
count.

### Recovery order: segment first, then WAL on top

The segment is settled state, the WAL is everything since. Loading them
the other way round means older segment data overwrites newer WAL
writes for any id in both, silently reverting real writes.

### Checkpoint order: write segment, then clear WAL

Crash between the two and the WAL just gets replayed on top of a segment
that may already contain the same records - harmless, since inserting an
id twice overwrites it with itself. The reverse order loses anything
that hadn't made it into the new segment yet.

## Search

### Squared L2, no sqrt

sqrt is monotonic so it doesn't change result ordering - if a < b then
sqrt(a) < sqrt(b). Leaving it out saves a sqrt per comparison and the
ranking is identical. Only worth taking if an actual distance value
needs reporting.

### Cosine similarity isn't supported yet

L2 works directly on raw vectors. Cosine needs normalized ones to mean
anything. It's easy to add later - normalize on insert and cosine
becomes a dot product - but it's not needed to get the index working,
so it's not in.

### A max-heap rather than sorting everything

Sorting all scores is O(n log n) and holds all n results in memory. The
heap is O(n log k) and only ever holds k. For k=10 against a million
vectors that's a real difference.

Max-heap rather than min-heap because the thing needing constant access
is the *worst* of the current best - that's what gets evicted when
something better turns up.

### Brute-force search stays permanently

It's not scaffolding. Once HNSW exists, this is what HNSW gets checked
against - an approximate index that disagrees with an exact scan is
wrong. It's also genuinely the right choice for small collections,
where building and walking a graph costs more than just scanning.

## Query planner

There's one real strategy right now, so the planner looks like overkill.
It's in early because the alternative is retrofitting a decision point
into every call site once HNSW lands - one function to change instead
of many.

The exact/approximate threshold is a placeholder. It gets set from real
benchmark numbers, not guessed at now.

## Things deliberately not done yet

- No batching of WAL flushes (correctness first)
- No checksums on records - a flipped bit inside a record that's still
  the right length passes silently today
- No compaction - the WAL only shrinks at checkpoint, segments are
  rewritten whole
- Single node only. No sharding, no replication.