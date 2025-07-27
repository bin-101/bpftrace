# bpftrace Issue #3549: `clear(..)` supports a sync mode

## 概要
- **Issue URL**: https://github.com/bpftrace/bpftrace/issues/3549
- **作成者**: sugarraysam
- **作成日**: 2024年10月29日
- **ステータス**: Open
- **ラベル**: Feature, good first issue
- **プロジェクト**: Standard Library Additions

## 問題の背景

現在のbpftraceでは、`clear()`関数は非同期削除のみをサポートしており、同期削除ができない。これにより、マップのローテーション処理において問題が発生する。

### 具体的な使用ケース
- `@curr`と`@prev`の2つのマップを使用
- 定期的に`@curr`のデータを`@prev`に移動
- この際、`@curr`を即座にクリアしたいが、現在の`clear()`は非同期のため問題となる

### 現在の回避策
```bpftrace
// 現在は手動でループして削除する必要がある
for (kv : @curr) {
    delete(@curr[$kv.0]);
}
```

## 提案された解決策

### 最終的な方針（2025年6月27日のコメント）
jordalgoによるコメント：
> After some offline discussion, we want to just make clear synchronous and not support a sync "mode". This could be done fairly easily in the map_sugar pass where we turn a call to `clear` into a loop over the map that calls delete.

**重要な注意点**：
- これは「オフラインでの議論の後」という表現で提案されている
- しかし、具体的にどのような議論が行われたかの詳細は不明
- この提案が確定的な決定なのか、単なる提案なのかは明確ではない
- 2025年6月27日以降、実装に関する具体的な進展は見られない

実装例：
```bpftrace
clear(@a);
```
↓変換後
```bpftrace
for ($kv : @a) { delete(@a, $kv.0); }
```

**現在の状況（2025年7月27日時点）**：
- Issue は依然として Open 状態
- "good first issue" ラベルが付いている
- 実装はまだ行われていない
- 最後の活動は2025年6月29日

## 技術的な議論

### パフォーマンスに関する議論
- **danobi**: カーネルサポートを追加してもループは避けられないため、ユーザーランドでの実装が適切
- **4ast**: カーネル側での最適化も可能だが、`map->max_entries`制限との兼ね合いで複雑になる

### 代替案の提案
**amscanne**による世代番号を使った最適化案：
- 各マップにグローバル世代番号を持たせる
- 値を`(gen, actual value)`として格納
- `clear`操作は`current_gen++`のみで`O(1)`実現
- ただし、個別操作のオーバーヘッドが増加

## 関連Issue
- #3028: 類似の問題だが、リングバッファのオーバーロードの懸念がより少ない
- #3551: Move assign maps（関連機能）

## 実装の方向性
- **難易度**: good first issue（初心者向け）
- **実装場所**: `map_sugar`パス
- **アプローチ**: `clear()`呼び出しを`for`ループ + `delete`に変換

## 調査日時
2025年7月27日 17:38

## コード調査結果（2025年7月27日 17:49追記）

### 1. AsyncAction列挙型の定義
`src/async_action.h`で`AsyncAction`列挙型が定義されている：

```cpp
enum class AsyncAction {
  // clang-format off
  printf      = 0,     // printf reserves 0-9999 for printf_ids
  printf_end  = 9999,
  syscall     = 10000, // system reserves 10000-19999 for printf_ids
  syscall_end = 19999,
  cat         = 20000, // cat reserves 20000-29999 for printf_ids
  cat_end     = 29999,
  exit        = 30000,
  print,
  clear,      // ← clear アクション
  zero,
  time,
  join,
  helper_error,
  print_non_map,
  strftime,
  watchpoint_attach,
  watchpoint_detach,
  skboutput,
  // clang-format on
};
```

### 2. AsyncHandlersクラス
`AsyncHandlers`クラスには`clear_map`メソッドが定義されている：

```cpp
class AsyncHandlers {
public:
  // ... 他のメソッド
  void clear_map(const void *data);
  // ...
};
```

### 3. コード生成部分（codegen_llvm.cpp）
`visit(Call &call)`メソッド内で`clear`関数の処理が行われている：

```cpp
} else if (call.func == "clear" || call.func == "zero") {
  auto elements = AsyncEvent::MapEvent().asLLVMType(b_);
  StructType *event_struct = b_.GetStructType(call.func + "_t",
                                              elements,
                                              true);

  auto &arg = call.vargs.at(0);
  auto &map = *arg.as<Map>();

  AllocaInst *buf = b_.CreateAllocaBPF(event_struct,
                                       call.func + "_" + map.ident);

  auto *aa_ptr = b_.CreateGEP(event_struct,
                              buf,
                              { b_.getInt64(0), b_.getInt32(0) });
  if (call.func == "clear")
    b_.CreateStore(b_.GetIntSameSize(static_cast<int64_t>(
                                         async_action::AsyncAction::clear),
                                     elements.at(0)),
                   aa_ptr);
  else
    b_.CreateStore(b_.GetIntSameSize(static_cast<int64_t>(
                                         async_action::AsyncAction::zero),
                                     elements.at(0)),
                   aa_ptr);

  int id = bpftrace_.resources.maps_info.at(map.ident).id;
  if (id == -1) {
    LOG(BUG) << "map id for map \"" << map.ident << "\" not found";
  }
  auto *ident_ptr = b_.CreateGEP(event_struct,
                                 buf,
                                 { b_.getInt64(0), b_.getInt32(1) });
  b_.CreateStore(b_.GetIntSameSize(id, elements.at(1)), ident_ptr);

  b_.CreateOutput(buf, getStructSize(event_struct), call.loc);
  return ScopedExpr(buf, [this, buf] { b_.CreateLifetimeEnd(buf); });
}
```

### 4. 現在の処理の流れ
1. `clear()`関数が呼ばれると、`AsyncEvent::MapEvent`構造体が作成される
2. `AsyncAction::clear`がアクションIDとして設定される
3. マップIDが設定される
4. `b_.CreateOutput()`でイベントが出力される（非同期処理）
5. ユーザー空間の`AsyncHandlers::clear_map()`メソッドが呼ばれる

### 5. 問題の核心
- 現在の実装では`clear()`は完全に非同期処理
- BPFプログラム内では実際のマップクリアは行われず、ユーザー空間への通知のみ
- これがIssue #3549で指摘されている同期化の問題の原因

### 6. 実装すべき変更点
Issue #3549の提案に従い、`map_sugar`パスで以下の変換を実装する必要がある：

```bpftrace
clear(@a);
```
↓変換後
```bpftrace
for ($kv : @a) { delete(@a, $kv.0); }
```

### 7. 関連ファイル
- `src/async_action.h` - AsyncAction列挙型の定義
- `src/ast/passes/codegen_llvm.cpp` - clear関数のコード生成
- `src/async_action.cpp` - AsyncHandlersの実装（要調査）
- `src/ast/async_event_types.h` - AsyncEventの定義（要調査）
- `src/ast/passes/map_sugar.cpp` - 実装が必要な箇所

## 実験結果（2025年7月27日 18:01追記）

### 実験の概要
`memo/clear_experiment/`ディレクトリで`clear()`の非同期性を確認する実験を実施。

### 重要な発見
1. **clear()は実際には同期的に動作している**
   - clear()実行直後（100ms後）にマップが完全に削除されている
   - 非同期であれば、直後にはまだデータが残っているはずだが、実際には即座に削除されている

2. **実験結果の詳細**
   - 最初の100ms時点でデータが確認できた（key1:100, key2:200, key3:300）
   - clear()実行後、200ms時点では全てのキーが削除済み
   - これは clear() が同期的に動作していることを示している

### 結論と考察
- 現在のbpftraceでは、`clear()`は実際には同期的に動作している可能性が高い
- Issue #3549で言及されている「非同期」の問題は、別の側面（ユーザー空間での処理、特定の条件下での動作など）を指している可能性がある
- コード上では`AsyncAction`として実装されているが、実際の動作は同期的である

### 今後の調査項目
1. AsyncActionの実装詳細を確認
2. ユーザー空間でのclear_map()処理を調査
3. 実際のIssue報告者の使用ケースを再現
4. 特定の条件下（大量データ、高負荷など）での動作確認
