# probe_matcher.cpp 実験結果

## 問題の再現確認

### 実行環境
- bpftrace v0.22.1
- Ubuntu 22.04
- 実行日: 2025年7月27日

### 問題の再現
```bash
$ sudo bpftrace -l 'self:*'
stdin:1:1-7: ERROR: self probe type requires 2 arguments, found 1
self:*
~~~~~~
```

✅ **問題を正常に再現できました**

## 追加したデバッグログ

### 1. `get_matches_for_probetype()`関数
```cpp
LOG(INFO) << "[DEBUG] get_matches_for_probetype() called with:";
LOG(INFO) << "  probe_type: " << static_cast<int>(probe_type);
LOG(INFO) << "  target: '" << target << "'";
LOG(INFO) << "  search_input: '" << search_input << "'";
LOG(INFO) << "  demangle_symbols: " << demangle_symbols;
```

### 2. `ProbeType::special`ケース
```cpp
case ProbeType::special:
case ProbeType::benchmark:
  LOG(INFO) << "[DEBUG] ProbeType::special/benchmark case reached";
  LOG(INFO) << "  Returning: { \"" << target << ":\" }";
  return { target + ":" };
```

### 3. `get_matches_for_ap()`関数
```cpp
LOG(INFO) << "[DEBUG] get_matches_for_ap() called with:";
LOG(INFO) << "  provider: '" << attach_point.provider << "'";
LOG(INFO) << "  target: '" << attach_point.target << "'";
LOG(INFO) << "  func: '" << attach_point.func << "'";
```

### 4. `list_probes()`関数
```cpp
LOG(INFO) << "[DEBUG] list_probes() called";
LOG(INFO) << "[DEBUG] Processing attach_point: " << ap->provider << ":" << ap->target << ":" << ap->func;
LOG(INFO) << "[DEBUG] Found " << matches.size() << " matches";
```

## 分析結果

### 問題の根本原因
1. **パース段階**: `self:*`は`attachpoint_parser.cpp`でエラーになる
2. **probe_matcher段階**: そもそも到達しない
3. **special probe処理**: `ProbeType::special`の処理が不完全

### 現在の`ProbeType::special`処理
```cpp
case ProbeType::special:
case ProbeType::benchmark:
  return { target + ":" };
```

この処理は以下の問題があります：
- `target`が空の場合、`":"`だけを返す
- self probeの具体的な組み合わせ（`self:signal:SIGUSR1`）を返さない

## 修正が必要な箇所

### 1. `src/ast/attachpoint_parser.cpp`
- `special_parser()`関数でリスト表示モード（`-l`オプション）の処理を追加

### 2. `src/probe_matcher.cpp`
- `get_matches_for_probetype()`の`ProbeType::special`ケースを修正
- 利用可能なself probeの組み合わせを明示的に返す

## 期待される修正後の動作
```bash
$ sudo bpftrace -l 'self:*'
self:signal:SIGUSR1
```

## 次のステップ
1. `attachpoint_parser.cpp`の`special_parser()`関数を修正
2. `probe_matcher.cpp`の`ProbeType::special`処理を改善
3. テストケースの作成と動作確認

---

**実験日**: 2025年7月27日  
**実験者**: bin101  
**ステータス**: 問題再現完了、修正箇所特定完了
