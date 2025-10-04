# `tests/runtime/stdlib` の役割

## 概要
- 標準ライブラリ (`src/stdlib/base.bt` など) に用意されているマクロの動作を、実際に bpftrace を起動して確認するテストケース集です。
- 例: `assert_macro` では `assert(false, "...")` によって適切なエラーメッセージが出るか検証。

## テストの構造
- 各ケースは以下のキーで構成されます。
  - `NAME`: テスト名。
  - `PROG`: 実際に bpftrace に渡す 1 行スクリプト。
  - `EXPECT` / `EXPECT_REGEX`: 標準出力に期待される文字列（直接一致 / 正規表現）。
  - `WILL_FAIL`: そのテストがエラー終了を想定している場合に記載します。

## 実行方法
- `./tests/runtime-tests.sh` から呼び出され、`tests/runtime/stdlib` のシナリオを順に処理します。
- 成功条件: `EXPECT`/`EXPECT_REGEX` に一致する出力が得られること。
- `WILL_FAIL` が指定されているケースは、エラー終了でもテスト成功とみなされます。

## 関連位置
- スクリプト: `tests/runtime/stdlib`
- 実行エンジン: `tests/runtime/engine/runner.py`
- 実行コマンド: `./tests/runtime-tests.sh`（CMake によりビルドディレクトリにもコピーされます）


## 実行ドライバ (`tests/runtime/engine/runner.py`) のポイント
- テストファイル（例: `tests/runtime/stdlib`）を読み込み、`NAME` / `PROG` / `EXPECT` などのキーを辞書形式に展開してから順次実行します。
- 実際には `bpftrace` バイナリを `subprocess.Popen` で起動し、標準出力を検査します。
- `stdlib/base.bt` の読み込みは bpftrace 本体に組み込まれているため、ここでは特別な処理は不要です。
- `runner.py` そのものを理解しやすいよう、`memo/tests/runtime/runner_commented.py` に最小限の動作をコメント付きで書き起こしてあります。
