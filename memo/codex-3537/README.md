# Issue #3537 memo

## 目的
`signal()` ビルトインにスレッド単位のシグナル送信を追加し、呼び出し側が

```bpftrace
signal(SIGTERM, current_tid);
```

のように書くと `BPF_FUNC_send_signal_thread` が使われるようになりました。

## 変更点の読み方
- `src/ast/passes/semantic_analyser.cpp` : 第 2 引数のバリデーションとヘルパー可否確認を実装。
- `src/ast/passes/codegen_llvm.cpp` / `src/ast/irbuilderbpf.cpp` : LLVM IR 生成時に `current_pid` と `current_tid` を切り分ける。
- `src/bpffeature.(h|cpp)` : 両ヘルパーの存在チェックを追加し、`bpftrace --info` で確認可能に。
- `tests/*` : セマンティックテストとコード生成テストを拡充。
- `docs/stdlib.md` : API ドキュメント更新。

## 補助資料
- `../src/signal_extension_notes.cpp` : セマンティック解析の擬似コード。
- `../tests/signal_extension_usage.cpp` : ヘルパー有無のチェックを簡略化したテスト例。

## 検証方法（手動）
1. `ninja bpftrace_test`（ユニットテスト）
2. `sudo ./build/tests/runtime-tests.sh`（必要に応じて）
3. `./build/src/bpftrace -e 'BEGIN { signal(9, current_tid); }'` 等で実行時エラーが出ないことを確認。

