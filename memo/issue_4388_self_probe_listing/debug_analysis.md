# Issue 4388 Debug Analysis

## Problem Summary
`bpftrace -l 'self:*'` should list available self probes (like `self:signal:SIGUSR1`) but instead returns an error:
```
stdin:1:1-7: ERROR: self probe type requires 2 arguments, found 1
self:*
~~~~~~
```

## Root Cause Analysis

### Current Code Flow
1. `main.cpp`: `-l` option creates a synthetic AST with `buildListProgram()`
2. `probe_matcher.cpp`: `get_matches_for_probetype()` is called for `ProbeType::special`
3. For `ProbeType::special`, it simply returns `{ target + ":" }` without expanding self probe types

### Key Files and Functions
- `src/probe_types.h`: Defines `self` as `ProbeType::special`
- `src/probe_matcher.cpp`: `get_matches_for_probetype()` - missing self probe expansion
- `src/probe_matcher.h`: Defines `SIGNALS = { "SIGUSR1" }`
- `src/bpftrace.cpp`: `add_probe()` - handles self probe attachment

### Expected Behavior
`bpftrace -l 'self:*'` should return:
```
special:signal:SIGUSR1
```

### Solution Approach
Need to modify `get_matches_for_probetype()` in `probe_matcher.cpp` to handle self probes specifically:
1. Detect when probe_type is `ProbeType::special` and target is "self"
2. Return available self probe types (currently only "signal")
3. For `self:signal:*`, return available signals (currently only "SIGUSR1")

## Next Steps
1. Add logging to understand current flow
2. Implement self probe expansion in probe_matcher.cpp
3. Test the fix
