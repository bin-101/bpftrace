# tests/semantic_analyser.cpp 解説メモ

## 目的
- セマンティック解析パス（`src/ast/passes/semantic_analyser.cpp`）の挙動をユニットテストで検証する。
- 関数呼び出しの型チェック、ビルトイン関数の使用条件、安全モード、警告等を細かく確認する。

## テストハーネスの概要
- `SemanticAnalyserHarness`：テンポラリの AST と `MockBPFtrace` を用意し、必要なパスを順番に適用 (`AllParsePasses` → `CreateSemanticPass` など)。
- 引数に `Mock`, `UnsafeMode`, `NoFeatures`, `Warning`, `Error` などを渡して、テストケースごとに状況を切り替える。
- `test(...)` 関数でスクリプト文字列を入力し、エラー/警告が期待値通りか検証。
  - `Error{ "..." }` を渡すと、その文字列を含むエラーメッセージが出力されることを期待します。
  - `Error{}`（空文字列）を渡した場合は、何かしらエラーが起きること自体を期待し、具体的なメッセージまでは検証しません。

## 主要な検証内容
- ビルトイン関数（`signal`, `printf`, `path` など）の引数チェック。
- map や配列の操作が正しく型チェックされるか。
- 非対応の attach point で警告/エラーになるか。
- `unsafe` モードや feature の有無で挙動が変わる場所（Issue #3537 関連では `signal` の helper チェックなど）。

## Issue #3537 での追加テスト
- `signal(1, current_pid)` / `signal(1, current_tid)` が許可されるか確認。
- `current_tid` を指定した場合、`BPF_FUNC_send_signal_thread` が利用可能かテスト（`MockBPFfeature` を用い、ヘルパー有無を切り替え）。
- `signal(1, foo)` のような無効なターゲットはエラーになるか。

## 参考
- 実装：`src/ast/passes/semantic_analyser.cpp`
- モック：`tests/mocks.h`（`MockBPFfeature` 等）
- コメント付き疑似コード：`memo/src/signal_extension_notes.cpp`
