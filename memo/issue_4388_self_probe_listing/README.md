# Issue #4388: `bpftrace -l 'self:*'` leads to strange error output

## 概要

このディレクトリには、bpftraceのGitHub issue #4388に関する調査結果をまとめています。

**Issue URL**: https://github.com/bpftrace/bpftrace/issues/4388

## 問題の要約

`bpftrace -l` コマンドでself probeをリストしようとすると、期待されるリスト出力ではなく、エラーメッセージが表示される問題です。

### 現在の問題のある動作
```bash
$ bpftrace -lv 'self:*'
stdin:1:1-7: ERROR: self probe type requires 2 arguments, found 1
self:*
~~~~~~
```

### 期待される動作
```bash
$ bpftrace -lv 'self:*'
special:signal:SIGUSR1
```

## ファイル構成

- `README.md` - このファイル（概要とナビゲーション）
- `issue_analysis.md` - issueの詳細分析と再現手順
- `technical_analysis.md` - 技術的な根本原因分析と修正方針
- `self_probe_explanation.md` - self probeの詳細説明と使用方法
- `sigusr1_mechanism_analysis.md` - SIGUSR1 self probeの内部実装詳細解析
- `ast_and_parsing_flow.md` - AST（抽象構文木）とパース処理の流れ解説
- `github_comment_draft.md` - GitHubでissueに取り組む意思表示のコメント案
- `probe_matcher_experiment.md` - probe_matcher.cpp実験計画
- `experiment_results.md` - 実験結果とデバッグログ分析
- `simple_self_probe.bt` - 基本的なself probeテストスクリプト
- `demo_self_probe.bt` - デモ用スクリプト

## 主な発見事項

### 1. 根本原因
- self probeは`ProbeType::special`として実装されている
- `probe_matcher.cpp`でspecial probeのリスト表示が適切に実装されていない
- `attachpoint_parser.cpp`でリスト表示モード（`-l`オプション）の特別処理が不足

### 2. 現在のサポート状況
- self probeで現在サポートされているのは`self:signal:SIGUSR1`のみ
- 他のシグナル（SIGHUP、SIGUSR2など）はサポートされていない

### 3. 修正の方向性
- `probe_matcher.cpp`の`get_matches_for_probetype()`関数を修正
- `attachpoint_parser.cpp`の`special_parser()`関数にリスト表示モードの処理を追加
- 利用可能なself probeの組み合わせを明示的に定義

## 技術的詳細

詳細な技術分析については以下のファイルを参照してください：

- **問題の詳細**: `issue_analysis.md`
- **技術的分析**: `technical_analysis.md`

## 関連するソースファイル

- `src/probe_matcher.cpp` - probe matching logic
- `src/ast/attachpoint_parser.cpp` - attach point parsing
- `src/probe_types.h` - probe type definitions
- `src/probe_types.cpp` - probe type utilities

## Issue分類

- **タイプ**: Bug
- **難易度**: Easy
- **新規参加者向け**: Good first issue
- **ステータス**: Open（2025年7月27日時点）

## 次のアクション

1. self probeでサポートされているtrigger/argumentの完全なリストを確認
2. 他のprobe typeのリスト表示実装を参考に修正案を作成
3. テストケースを作成して動作確認
4. プルリクエストの作成

---

**調査日**: 2025年7月27日  
**調査者**: bin101  
**bpftraceバージョン**: v0.23.4（issue報告時）
