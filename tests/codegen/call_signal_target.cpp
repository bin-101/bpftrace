#include "common.h"

namespace bpftrace::test::codegen {

TEST(codegen, call_signal_target_none)
{
  test("k:f { signal(8); }", NAME, false);
}

TEST(codegen, call_signal_target_pid)
{
  test("k:f { signal(8, current_pid); }", "call_signal_target_none", false);
}

TEST(codegen, call_signal_target_tid)
{
  test("k:f { signal(8, current_tid); }", NAME, false);
}

} // namespace bpftrace::test::codegen
