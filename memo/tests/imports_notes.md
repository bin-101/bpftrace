# tests/imports.cpp 解説メモ

## 目的
- bpftrace の `import` 機能が正しく動作するかを単体テストで確認する。
- 標準ライブラリ (`stdlib/base.bt`) の暗黙読み込みや、ユーザー用ディレクトリの優先順位・権限チェックをカバー。

## テストの構造
- GoogleTest を使用。
- `TempDir`/`TempFile` を使って一時的なディレクトリ・ファイルを作成し、実際のファイルシステム操作に近い状態で検証。
- `ast::PassManager` に `AllParsePasses` を組み込み、`Imports` 状態を取り出して `imports.scripts.contains(...)` で結果を確認。

## 主なケース
1. **`stdlib_works`**
   - `begin {}` を実行したとき、暗黙に `stdlib/base.bt` が読み込まれているか確認。
   - 明示的に `import "stdlib";` を行っても同じ結果になるか検証。

2. **`stdlib_implicit_rules`**
   - ユーザーが `stdlib` ディレクトリを用意した場合はそちらが優先される（`stdlib/foo.bt` があると `stdlib/base.bt` は読み込まれない）。
   - 明示的な import が無い場合は従来どおり builtin の `stdlib/base.bt` を使用。

3. **`world_writable_ignored`**
   - world-writable (0777) なディレクトリはセキュリティ上の理由で import できないことを確認。
   - ただし `lib/foo.bt` のような直接指定であれば許可されるケースもチェック。

4. **`import_ordering`**
   - import パスを複数設定した場合、先にヒットしたディレクトリを優先する挙動を確認。

## メモ
- 実際に `ImportInternalScriptsPass` や `Driver::add_stdlib()` が `stdlib/base.bt` を組み込むため、標準ライブラリの暗黙読み込みが可能になっています。
- これらのテストは `bpftrace` の import 仕様が変わった際に不具合が混入しないよう守りの役割を担っています。

