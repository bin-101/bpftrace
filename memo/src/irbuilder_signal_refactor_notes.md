# IRBuilderBPF::CreateSignal に追加引数を持たせるか、新関数を用意するか

## 1. 現状
- `IRBuilderBPF::CreateSignal(sig, loc)` が既存 (`BPF_FUNC_send_signal` 用)。
- Issue #3537 で `CreateSignalThread(sig, loc)` を新設し、`BPF_FUNC_send_signal_thread` を呼ぶよう実装。

## 2. `CreateSignal` に引数を追加する案
### メリット
- 似た処理が1箇所にまとまるため、重複コードを避けられる。
- 将来的に別ターゲットが増えても引数の拡張で対応できそう。
### デメリット
- 呼び出し側に `enum class SignalTarget { CurrentPid, CurrentTid };` 等を導入する必要がある。
- 既存コードとの互換性（引数の追加によるオーバーロード整理）を慎重に扱う必要がある。
- `CreateSignal(...)` の内部が条件分岐で複雑になる。

## 3. 新しい関数を作る案（現行）
### メリット
- 呼び出し側で意図が明確（`CreateSignalThread`）。
- 既存の `CreateSignal` のシグネチャを変えないため安全。
- 実装もほぼコピーで済むため変更が小さい。
### デメリット
- 似たコード（Helper call）を2箇所維持する必要がある。
- ターゲットが増えるたびに関数が増え、命名が煩雑になる恐れ。

## 4. 結論
- 今回のようにターゲットが2種類だけの場合は、後方互換性の観点から **新しい関数を用意する現行案の方が安全**。
- 将来ターゲットがさらに増えるような要件が出た場合は、`CreateSignal(sig, loc, SignalTarget target)` のように統合することを検討すると良い。

