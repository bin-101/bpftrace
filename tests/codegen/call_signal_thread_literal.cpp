#include "common.h"

namespace bpftrace::test::codegen {

TEST(codegen, call_signal_thread_literal)
{
  test("k:f { signal(8, current_tid); }", NAME, false);
}

} // namespace bpftrace::test::codegen
