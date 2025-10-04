# `src/stdlib/base.bt` 概要メモ

`base.bt` は bpftrace が提供する標準ライブラリの中心的なスクリプトで、
以下のような要素で構成されています（実際は `.bt` = bpftrace スクリプト形式）。

1. **マクロ定義** – `print()`, `exit()` などのラッパを提供。
2. **組み込み関数のラッパ** – `kstack()`, `ustack()`, `signal()` など。
3. **コメントによるドキュメント** – `// :function` コメントで CLI の `--info` や `docs/stdlib.md` と同期。

今回の Issue #3537 に関わるのは 3 項の `signal()` ブロック。下記にポイントをまとめます。

## `signal()` ブロックの抜粋と解説

```bpftrace
// :function signal
// :variant void signal(const string sig)
// :variant void signal(uint32 signum)
// :variant void signal(const string sig, signal_target target)
// :variant void signal(uint32 signum, signal_target target)
//
// **unsafe**
//
// **Kernel** 5.3 (`current_pid`), 5.5 (`current_tid`)
//
// This utilizes the BPF helpers `bpf_send_signal` and `bpf_send_signal_thread`
//
// Probe types: k(ret)probe, u(ret)probe, USDT, profile
//
// Send a signal to the process or thread being traced.
// The signal can either be identified by name, e.g. `SIGSTOP` or by ID, e.g. `19` as found in `kill -l`.
// The optional second argument selects the target (`current_pid` is the default).
// `signal_target` accepts `current_pid` or `current_tid`.
```

### コメントの読み方
- `:function` / `:variant` – CLI ドキュメント生成用。`--info` に反映されます。
- `**unsafe**` – `bpftrace --unsafe` が必要。
- `**Kernel**` – 利用する BPF helper の導入バージョンを明示。
- 直下の文面 – 実装方針の説明。

### このブロックが行うこと
`signal()` 自体は `.bt` 側ではマクロ/ラッパ定義をしていません。
実際の処理は C++ 側（`src/ast/passes/codegen_llvm.cpp` や
`src/ast/irbuilderbpf.cpp`）にあり、上記コメントは**利用方法を利用者に分かりやすく伝える役割**を担っています。

## 代表的な他セクション（理解の参考）

- `nsecs()` ブロックでは `TimestampMode`（`monotonic`, `boot`, etc.）を指定するパターンを定義。
- `kstack()` / `ustack()` ブロックはさらに詳細なサンプルコードを掲載し、
  使い方や挙動をコメントベースで説明。

同様に `signal()` でも **「第 2 引数でターゲットを選択する」「利用可能な helper とカーネルバージョン」** を明記しており、bpftrace ユーザー向けの最初の参照元になっています。

## 深掘りのためのリンク
- 実際の実装: `src/ast/passes/codegen_llvm.cpp`（`call.func == "signal"`）
- Helper 検出: `src/bpffeature.{h,cpp}` (`has_helper_send_signal` 等)
- セマンティック解析: `src/ast/passes/semantic_analyser.cpp`

`memo/src/signal_extension_notes.cpp` と `memo/tests/signal_extension_usage.cpp`
も合わせて読むと、コードフローとテスト観点が掴みやすくなります。


## `base.bt` を利用する C++ 側の主な箇所

1. **`src/driver.cpp` の `Driver::add_stdlib()`**
   - bpftrace 起動時に標準ライブラリ `.bt` ファイル群を読み込み、`base.bt` もここで AST に追加されます。

2. **`src/ast/passes/import_internal_scripts.cpp`**
   - `@std::__base` として内部スクリプトを登録する処理があり、最終的にユーザースクリプトの AST とマージされます。

3. **その後の各パス（セマンティック解析、コード生成など）**
   - 以降は通常のスクリプトと同様に扱われ、`signal()` などのビルトインをユーザースクリプトから呼び出せるようになります。

> 参考: `Driver::run()` → `add_stdlib()` → `ImportInternalScriptsPass` → （ユーザー AST と結合）という流れ。

4. **テストとドキュメント生成**
   - `tests/imports.cpp` では、暗黙に読み込まれる標準ライブラリ（`stdlib/base.bt`）の挙動がユニットテストで検証されています。
   - `.github/workflows/stdlib-docs.yml` と `scripts/generate_stdlib_docs.py` は、`base.bt` の `// :function` コメントから `docs/stdlib.md` を自動生成し、CI でドキュメント更新漏れを検出する仕組みです。

