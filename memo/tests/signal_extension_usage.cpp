// signal_extension_usage.cpp
// Issue #3537 の挙動を簡単に再現するための参考テストコード（実行はされません）。
// tests/semantic_analyser.cpp や tests/codegen 以下の実テストを読む前の導入として使えます。

#include <cassert>
#include <string>

namespace mock {

// 擬似的な helper 利用可否を表現する構造体。
struct FeatureState
{
  bool has_send_signal = true;
  bool has_send_signal_thread = true;
};

// signal("SIGTERM", current_tid) のチェックを模した関数。
static bool validate_signal_thread(const FeatureState &feature)
{
  if (!feature.has_send_signal_thread)
    return false; // 実装ではエラーメッセージ付きで失敗する。

  return true;
}

// signal(9) のチェックを模した関数。
static bool validate_signal_process(const FeatureState &feature)
{
  if (!feature.has_send_signal)
    return false;

  return true;
}

} // namespace mock

int main()
{
  mock::FeatureState ok_state;
  assert(mock::validate_signal_thread(ok_state));
  assert(mock::validate_signal_process(ok_state));

  mock::FeatureState missing_thread{ .has_send_signal = true, .has_send_signal_thread = false };
  assert(!mock::validate_signal_thread(missing_thread));

  mock::FeatureState missing_proc{ .has_send_signal = false, .has_send_signal_thread = true };
  assert(!mock::validate_signal_process(missing_proc));

  return 0;
}
