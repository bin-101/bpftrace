# tests/codegen ディレクトリ解説メモ

## 目的
- `tests/codegen/*.cpp` は、bpftrace が生成する LLVM IR が期待通りかを検証するユニットテストです。
- それぞれのテストファイルは、簡単な bpftrace スクリプトを入力して `codegen/test(...)` を呼び出し、出力された IR が `tests/codegen/llvm/*.ll` に保存された期待値と一致するか確認します。

## 構成
- `tests/codegen/*.cpp`: 各ビルトインや構文に対応したテスト（例: `call_signal_literal.cpp`）。
- `tests/codegen/llvm/*.ll`: 上記テストが期待する LLVM IR。`scripts/generate_stdlib_docs.py` と同様、差分チェックで更新漏れを検出する仕組み。
- `tests/codegen/common.h`: テスト実行の共通処理（AST 生成→Pass 適用→IR 出力→比較）。

## 実行条件
- CMake で `ENABLE_TEST_VALIDATE_CODEGEN` が有効になっている場合、LLVM 18 + GCC の環境で `codegen` テストが実行されます。
- 環境変数 `BPFTRACE_UPDATE_TESTS=1` をセットすると、比較ではなく `.ll` ファイルを更新するモードになります。

## Issue #3537 関連
- `tests/codegen/call_signal_thread_literal.cpp` を追加し、`signal(8, current_tid)` が `BPF_FUNC_send_signal_thread` を呼ぶ IR を生成するか確認します。
- 対応する IR は `tests/codegen/llvm/call_signal_thread_literal.ll` に保存されています。

