# tests/mocks.h 解説メモ

## 子要素の概要
- **MockBPFtrace**: 実際の `BPFtrace` に近いモック。セマンティックテスト等で利用。
- **MockBPFfeature**: `BPFfeature` の派生クラスで、ヘルパー可否をスイッチできる。
- **MockChildProc / MockProcMon / MockUSDTHelper** など: 各種モック実装。

## MockBPFfeature の役割
- コンストラクタで `has_send_signal`, `has_send_signal_thread` などを `std::optional<bool>` で初期化。
- `set_helper_send_signal(bool)`, `set_helper_send_signal_thread(bool)` を使って helper の有無をテストケース毎に制御可能。
- Issue #3537 のテストでは、このモックを通じて `BPF_FUNC_send_signal` / `_thread` の有無を切り替え、エラーメッセージが想定通り出るかを確認。

## 利用方法
- `tests/semantic_analyser.cpp` で `MockBPFtrace` と一緒に使われる。
- 例: `auto mock = get_mock_bpftrace();` した後、`static_cast<MockBPFfeature *>(mock->feature_.get())->set_helper_send_signal_thread(false);`

