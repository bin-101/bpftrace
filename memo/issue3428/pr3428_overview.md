# PR #3428: Make bpftrace work correctly inside containers with PID namespacing

## 基本情報
- **PR番号**: #3428
- **タイトル**: Make bpftrace work correctly inside containers with PID namespacing
- **作成者**: ajor
- **作成日**: 2024年8月29日
- **マージ日**: 2025年1月15日
- **状態**: マージ済み

## 問題の概要
bpftraceをPIDネームスペース内（コンテナ内など）で実行した際に、`pid`、`tid`、`ustack`ビルトインが正しく動作しない問題があった。

### 従来の問題
- `pid`、`tid`ビルトインが常に初期（グローバル）ネームスペースのPIDを返していた
- これにより、bpftraceランタイムから見た正しいプロセスに対応しなくなっていた
- bpftraceランタイムはネームスペース化されたPIDを見るため、不整合が発生

## 技術的背景

### eBPFヘルパー関数の特性
1. **`bpf_get_current_pid_tgid`**
   - 常にルートネームスペースのPIDを返す
   - 従来bpftraceが使用していたヘルパー

2. **`bpf_get_ns_current_pid_tgid`**
   - 指定されたネームスペース内のPIDを返す
   - そのネームスペース内で実行されているプロセスに対してのみ有効

### 解決アプローチ
bpftraceが実行されているネームスペースに応じて、適切なヘルパー関数を選択する仕組みを実装。

## 動作比較表

### 従来の動作（常に bpf_get_current_pid_tgid を使用）
```
        \ target |                |
bpftrace \       | root namespace | child namespace
-----------------+----------------+-----------------
root namespace   | YES            | YES
child namespace  | N/A            | NO
```

### 新しい動作（適切なヘルパーを選択）
```
        \ target |                |
bpftrace \       | root namespace | child namespace
-----------------+----------------+-----------------
root namespace   | YES            | YES
child namespace  | N/A            | YES
```

## 制限事項
- ターゲットプロセスが別のネームスペースで実行されており、bpftraceが異なるネームスペース（ただしターゲットのネームスペースが見える）から実行されている場合は、まだ正しいPIDを取得できない
- この問題の完全な解決にはカーネル側の変更が必要

## 影響を受ける機能
- `pid` ビルトイン
- `tid` ビルトイン  
- `ustack` ビルトイン

## 次のステップ
このPRの詳細な実装を理解するために、以下を調査する予定：
1. 具体的なコード変更
2. テストケース
3. 実装の詳細
