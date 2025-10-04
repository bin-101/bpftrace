// signal_extension_notes.cpp
// Issue #3537 の実装を読み解くためのメモ用疑似コードです。
// 実際の bpftrace ビルドには含まれず、振る舞いを把握するための参考資料となります。

#include <string>

namespace bpftrace {
namespace ast {

class BPFfeature; // 実装本体では src/bpffeature.h を参照する。

// セマンティック解析で呼ばれるロジックの抜粋イメージ
// src/ast/passes/semantic_analyser.cpp を簡略化したもの。
static bool validate_signal_args(const std::string &target_literal)
{
  // 第 2 引数は "current_pid" か "current_tid" のみを許容する。
  if (target_literal == "current_pid" || target_literal == "current_tid")
    return true;

  return false; // それ以外はエラー扱い。
}

// 実際の実装では Call オブジェクトを介して helper 可否を問い合わせる。
static bool can_use_signal_helper(bool target_thread,
                                  const BPFfeature &feature,
                                  std::string &error)
{
  if (target_thread && !feature.has_helper_send_signal_thread())
  {
    error = "BPF_FUNC_send_signal_thread not available for your kernel version";
    return false;
  }

  if (!target_thread && !feature.has_helper_send_signal())
  {
    error = "BPF_FUNC_send_signal not available for your kernel version";
    return false;
  }

  return true;
}

} // namespace ast
} // namespace bpftrace

// 上記の関数はテストやドキュメントから参照するためのサンプルです。
// 詳細は実ファイル: src/ast/passes/semantic_analyser.cpp を確認してください。
