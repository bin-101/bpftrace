# Issue 3537 – signal() 拡張対応ログ

## 2025-03-07 進捗メモ
- master ブランチへ切り替え済み。
- 仕様確認：`signal(signum, target)` 形式で `target` は `process`(既定) / `thread` を想定。
- これからの作業方針：
  1. `SemanticAnalyser` の定義を更新し、第2引数を許容・検証する。
  2. `Identifier` 訪問処理に `process` / `thread` を追加。
  3. `BPFfeature` に `send_signal` / `send_signal_thread` のヘルパー検出を追加。
  4. `codegen` / `IRBuilder` で `BPF_FUNC_send_signal_thread` を呼び分ける。
  5. 付随ドキュメント（`src/stdlib/base.bt` や docs）とテスト（semantic / codegen）を更新。

- `src/ast/passes/semantic_analyser.cpp` にて `signal` の第2引数を受け入れ、`process`/`thread` 判定と helper 利用可否チェックを追加。
- `src/ast/passes/codegen_llvm.cpp` と `src/ast/irbuilderbpf.(h|cpp)` を更新し、`signal(..., thread)` で `BPF_FUNC_send_signal_thread` を呼び出すように実装。
- `src/bpffeature.(h|cpp)` と `tests/mocks.h` を拡張し、`send_signal` / `send_signal_thread` helper の検出・モック制御をサポート。
- ドキュメント (`src/stdlib/base.bt`, `docs/stdlib.md`, `memo/docs/stdlib.md`) を新インターフェース仕様に合わせて更新。
- 単体テスト（semantic/codegen）を追加・拡張し、新しいターゲット引数と helper 有無の挙動を検証。
- signal ターゲット識別子を current_pid/current_tid に更新。
