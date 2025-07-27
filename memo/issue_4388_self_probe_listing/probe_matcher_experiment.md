# probe_matcher.cpp 実験計画

## 目的
`probe_matcher.cpp`の動作を理解し、特に`ProbeType::special`（self probe）の処理を詳しく調査する。

## 実験内容

### 1. ログ追加箇所
以下の関数にログを追加：
- `get_matches_for_probetype()` - プローブタイプ別の処理
- `get_matches_for_ap()` - AttachPointからの検索入力生成
- `list_probes()` - プローブリスト表示

### 2. 追加するログ
- 関数の入力パラメータ
- 条件分岐の結果
- 返り値

### 3. テストケース
- `bpftrace -l 'self:*'`
- `bpftrace -l 'self:signal:*'`
- `bpftrace -l 'kprobe:*'` (比較用)

## 期待される発見
- `ProbeType::special`の処理が不完全であることの確認
- `target + ":"`の返り値が適切でないことの確認
- 修正すべき箇所の特定
