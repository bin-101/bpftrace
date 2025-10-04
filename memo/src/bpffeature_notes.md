# src/bpffeature.cpp 解説メモ

## 役割
- カーネルが提供する BPF helper や機能の有無を動的に検出し、`BPFtrace` から参照できるようにするクラス (`BPFfeature`) の実装。
- `bpf_send_signal` などの helper を実際にロードしてみることで可否を判定し、結果を `std::optional<bool>` でキャッシュ。
- `BPFtrace --info` で確認できる機能一覧 (`report()` メソッド) もここで生成される。

## 主な関数
- `define_helper_test` マクロ（`DEFINE_HELPER_TEST`）: `has_helper_<name>()` を自動生成し、初回だけ helper を試行ロードする仕組みを構築。
- `detect_helper(...)`: 指定した BPF helper を含む短いプログラムを `bpf_prog_load` し、成功すれば利用可能と判断。
- `report()`: `has_helper_*()` の結果などをまとめて文字列として返し、`--info` に表示される。
- 今回の Issue #3537 では `send_signal` / `send_signal_thread` の検出が追加され、`report()` にも反映された。

## 使われ方
- セマンティック解析 (`src/ast/passes/semantic_analyser.cpp`) から `feature_->has_helper_send_signal()` 等を呼び出し、カーネルバージョンの条件に沿ったエラーメッセージを表示。
- テストでは `MockBPFfeature` を使うことで `has_helper_send_signal_thread()` の戻り値を任意に切り替え、エラーハンドリングを検証。

