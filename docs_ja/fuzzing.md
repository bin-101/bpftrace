# bpftraceのファジング

このドキュメントは`bpftrace`開発者向けです。

## はじめに

ファジングは、プログラムのバグを自動的に見つける手法です。ファジングでは、ファザーがプログラムの入力を生成して与え、プログラムがクラッシュするかどうかを観察します。最も一般的に使用されるファジング手法は、カバレッジ（プログラムのどの部分が実行されるか）情報を使用して効率的に入力を生成するグレーボックスファジングと呼ばれます。

ファジングは、ファジングの対象に応じて2つのタイプに分けることができます：AFLのようにプログラム全体をファジングの対象とするものと、libFuzzerのように特定の関数を対象とするものです。前者の場合、ファザーがプログラムの入力を生成して供給するため、プログラムを修正する必要がありません。これは大きなプログラムに対して常に効率的とは限りませんが、それでも多くのバグを見つけることができます。後者は、ファザーが直接関数を対象とするため、ファジングする関数に対して効率的ですが、ファザーと関数を接続するためのグルーコードを書く必要があります。

## オプション

### `BPFTRACE_MAX_AST_NODES`環境変数

ファジングを行う際は、ASTノードの数を制限することが重要です。そうしないと、ファザーがスタックオーバーフローを引き起こす非常に長いプログラムを生成し続ける可能性があります。`BPFTRACE_MAX_AST_NODES`環境変数は、ASTノードの最大数を制御します。

## AFLによるファジング

開始前に、[AFL][AFL]または[AFLPlusPlus][AFL++]のドキュメントを読むことを強く推奨します。

### セットアップ

ファジングのセットアップは`nix`に依存しています。[nix手順](./nix.md)を参照してください。

### コンパイル

AFLを使用するには、AFLコンパイラでプログラムをコンパイルする必要があります。`nix`を使用している場合は、開発シェルに入り、以下のようにビルドします：

```
nix develop #.bpftrace-fuzz
CC=afl-clang-fast CXX=afl-clang-fast++ cmake -B build-fuzz -DCMAKE_BUILD_TYPE=Debug -DBUILD_ASAN=1
```

その後、ツリーをビルドするために：
```
cd build-fuzz && AFL_USE_ASAN=1 make -j$(nproc)
```

アドレスサニタイザーは大量のメモリを消費する可能性があることに注意してください。それなしでファジングしたい場合は、`AFL_USE_ASAN`と`-DBUILD_ASAN`を削除してください。

### 実行

AFLは効率的なファジングのためにいくつかの設定を推奨しています：

```
echo core | sudo tee -a /proc/sys/kernel/core_pattern
cd /sys/devices/system/cpu
echo performance | sudo tee cpu*/cpufreq/scaling_governor
```

その後、ファジングを開始しましょう！AFLとアドレスサニタイザーには多くの設定があるため、詳細についてはドキュメントを読んでください。現在推奨されているファザーの実行方法は、`--test=codegen`モードを使用し、オーバーライドを提供することです：

```
AFL_NO_AFFINITY=1 \
ASAN_OPTIONS=abort_on_error=1,symbolize=0 \
BPFTRACE_BTF= \
BPFTRACE_MAX_AST_NODES=200 \
BPFTRACE_AVAILABLE_FUNCTIONS_TEST= \
afl-fuzz -a text -M 0 -m none -i ./input -o ./output -t 3000 -- \
     src/bpftrace --test=codegen @@ 2>/dev/null
```

上記で、`-i`は入力ディレクトリを指定し、`-o`は出力ディレクトリを指定します。入力ディレクトリには、ファジングを開始するための何かを置く必要があります。最も簡単な例は`echo a > input/a`です。より洗練された入力は、ソースディレクトリ、テスト、またはその他の場所からサンプル`bpftrace`プログラムを使用して作成できます。プログラムクラッシュを引き起こす入力が見つかった場合、`output/crashes`にそれらが含まれます。

各実行のタイムアウト（ミリ秒）は`-t`で提供されます。

最後に、'@@'はファザーによって生成された入力ファイルに置き換えられます。

## 発見されたバグ

### AFL
- [#1623](https://github.com/bpftrace/bpftrace/pull/1623)
- [#1619](https://github.com/bpftrace/bpftrace/pull/1619)
- [#1580](https://github.com/bpftrace/bpftrace/pull/1580)
- [#1573](https://github.com/bpftrace/bpftrace/pull/1573)
- [#1572](https://github.com/bpftrace/bpftrace/pull/1572)
- [#1570](https://github.com/bpftrace/bpftrace/pull/1570)
- [#1568](https://github.com/bpftrace/bpftrace/pull/1568)
- [#1286](https://github.com/bpftrace/bpftrace/pull/1286)
- [#1245](https://github.com/bpftrace/bpftrace/pull/1245)
- [#1234](https://github.com/bpftrace/bpftrace/pull/1234)
- [#1229](https://github.com/bpftrace/bpftrace/pull/1229)
- [#1224](https://github.com/bpftrace/bpftrace/pull/1224)
- [#1222](https://github.com/bpftrace/bpftrace/pull/1222)
- [#1221](https://github.com/bpftrace/bpftrace/pull/1221)
- [#1210](https://github.com/bpftrace/bpftrace/pull/1210)
- [#1205](https://github.com/bpftrace/bpftrace/pull/1205)

### libFuzzer
- [#1653](https://github.com/bpftrace/bpftrace/pull/1653)
- [#1650](https://github.com/bpftrace/bpftrace/pull/1650)
- [#1622](https://github.com/bpftrace/bpftrace/pull/1622)
- [#1621](https://github.com/bpftrace/bpftrace/pull/1621)

[AFL]: https://github.com/google/AFL
[AFL++]: https://github.com/AFLplusplus/AFLplusplus
