# Issue #4388: `bpftrace -l 'self:*'` leads to strange error output

## 概要
- **Issue URL**: https://github.com/bpftrace/bpftrace/issues/4388
- **報告者**: VorpalBlade
- **報告日**: 2025年7月27日
- **ラベル**: Bug, difficulty: easy, good first issue
- **ステータス**: Open

## 問題の詳細

### 現在の動作
`bpftrace -l` コマンドでself probeをリストしようとすると、期待されるリスト出力ではなく、エラーメッセージが表示される。

#### 再現手順と結果

1. **基本的なワイルドカード使用**
   ```bash
   $ bpftrace -lv 'self:*'
   stdin:1:1-7: ERROR: self probe type requires 2 arguments, found 1
   self:*
   ~~~~~~
   ```

2. **2つのワイルドカード使用**
   ```bash
   $ bpftrace -lv 'self:*:*'
   stdin:1:1-9: ERROR: * is not a supported trigger
   self:*:*
   ~~~~~~~~
   ```

3. **signal probeでのワイルドカード**
   ```bash
   $ bpftrace -lv 'self:signal:*'
   stdin:1:1-14: ERROR: * is not a supported signal
   self:signal:*
   ~~~~~~~~~~~~~
   ```

4. **無効なシグナル名**
   ```bash
   $ bpftrace -lv 'self:signal:SIGUSR'
   stdin:1:1-19: ERROR: SIGUSR is not a supported signal
   self:signal:SIGUSR
   ~~~~~~~~~~~~~~~~~~
   ```

5. **有効なシグナル名（SIGUSR1のみサポート）**
   ```bash
   $ bpftrace -lv 'self:signal:SIGUSR1'
   special:signal:
   ```

### 期待される動作
- 他のprobe typeと同様に、`-l`オプションでリスト出力が動作すべき
- 利用可能なself probeの一覧が表示されるべき
- 特にsignalについては、サポートされているシグナル（現在はSIGUSR1のみ）の一覧が表示されるべき

## 技術的な分析

### 現在のself probeサポート状況
- **signal**: SIGUSR1のみサポート
- 他のself probe typeについては要調査

### 問題の原因
- リスト機能（`-l`オプション）がself probe typeに対して適切に実装されていない
- ワイルドカード（`*`）の処理がself probeで正しく動作していない
- パーサーがリスト表示モードとスクリプト実行モードを区別していない可能性

## 環境情報
- **OS**: Linux 6.15.7-zen1-1-zen (Arch Linux)
- **bpftrace version**: v0.23.4
- **LLVM**: 20.1.6

## 関連するコメント
- jordalgoによると、現在SIGUSR1のみがサポートされており、リストにはそれだけが表示されるべき
- VorpalBladeは他のシグナル（SIGHUP、SIGUSR2など）がサポートされていないことを確認

## 修正の方向性
1. `-l`オプション使用時のself probeの処理を修正
2. サポートされているself probe typeとその引数の一覧表示機能を実装
3. ワイルドカード処理の改善

## 分類
- **難易度**: Easy
- **新規参加者向け**: Good first issue
- **タイプ**: Bug
