# Issue #4412: USDTテストプログラムのlibbpf/usdtライブラリへの移行

## 概要
現在のbpftraceプロジェクトのUSDTテストプログラムは、SystemTapのsdt.hを使用している。
これをlibbpf/usdtライブラリに移行する必要がある。

## なぜ移行するのか？

### 1. 依存関係の簡素化
- **現在**: SystemTap SDTは`systemtap-sdt-devel`パッケージが必要
- **移行後**: libbpf/usdtは単一ヘッダーファイルのみ、依存関係なし
- **メリット**: ビルド環境の構築が簡単になり、CI/CDでの問題が減る

### 2. メンテナンス性の向上
- **現在**: SystemTapプロジェクトの一部として管理されており、bpftraceプロジェクトでは制御できない
- **移行後**: 単一ファイルをプロジェクト内で管理、必要に応じてカスタマイズ可能
- **メリット**: バージョン管理が容易、問題発生時の対応が迅速

### 3. API の使いやすさ
- **現在**: セマフォの使用が複雑（全ファイルで統一する必要がある）
- **移行後**: セマフォありなしを自由に混在可能、より直感的なAPI
- **メリット**: テストコードの記述が簡単になり、保守性が向上

### 4. 将来性
- **現在**: SystemTapは現在も開発継続中だが、BPFエコシステムへの移行が進んでいる
- **移行後**: libbpf/usdtは新しく設計されたライブラリで、継続的な改善が期待できる
- **メリット**: 長期的な技術的負債の軽減

#### SystemTapの開発状況（2025年現在）
- **公式サイト**: https://sourceware.org/systemtap/ （現在も稼働中）
- **開発状況**: 現在も開発は継続されているが、活動は以前より低下
- **GitHubでの活動**: 19個のパブリックリポジトリが存在、最新の更新は2025年3月
- **業界動向**: LinuxトレーシングはBPF/eBPFエコシステムへの移行が主流
- **SystemTap vs BPF**: 
  - SystemTapは独自のランタイムとカーネルモジュールが必要
  - BPFはカーネル組み込みで、より軽量で安全

### 5. bpftraceプロジェクトの方針
- bpftraceはlibbpfエコシステムとの統合を進めている
- libbpf/usdtはlibbpfプロジェクトの一部として開発されている
- **メリット**: プロジェクト全体の技術スタックの一貫性

### 6. パフォーマンス
- 両方とも実行時オーバーヘッドはほぼゼロ
- libbpf/usdtはより最適化されたコード生成が期待できる
- **メリット**: わずかながらパフォーマンス向上の可能性

## 背景知識

### SystemTapとは
- SystemTapは、Linuxカーネルとユーザースペースアプリケーションの動的トレーシングツール
- カーネルやアプリケーションの実行時の動作を監視・分析するためのフレームワーク
- スクリプト言語を使ってトレースポイントを定義し、データを収集できる

### sdt.h（SystemTap SDT）とは
- SDT = Statically Defined Tracepoints（静的定義トレースポイント）
- SystemTapプロジェクトの一部として提供されるヘッダーファイル
- ユーザースペースアプリケーションにUSDTを埋め込むためのマクロを提供
- `DTRACE_PROBE*`マクロを使ってトレースポイントを定義

### USDTとは
- USDT = User Statically-Defined Tracepoints
- ユーザースペースアプリケーションに静的に定義されるトレースポイント
- アプリケーションの特定の場所にマーカーを埋め込み、外部のトレーシングツールから観測可能にする
- 実行時オーバーヘッドはほぼゼロ（NOPインストラクション）

## 現在の実装

### 重要な発見: libbpf/usdt.hは未使用
調査の結果、以下のことが判明：

1. **bpftraceの`src/usdt.h`**: これはUSDTプローブを検出・管理するためのC++クラス（bpftrace内部用）
2. **libbpf/usdt.hマクロ**: テストプログラムでは使用されていない（`USDT(`、`USDT_WITH_SEMA(`等のマクロが見つからない）
3. **tests/include/**: `sdt.h`と`sdt-config.h`のみ存在、libbpf/usdt.hは未配置

つまり、Issue #4412の移行作業は実際にlibbpf/usdt.hを導入する作業から始める必要がある。

### SystemTap SDTのリポジトリ情報
- **公式リポジトリ**: https://sourceware.org/git/?p=systemtap.git
- **ファイルパス**: `includes/sys/sdt.h`
- **ライセンス**: CC0 (Public Domain)
- **説明**: SystemTapプロジェクトの一部として管理されている

### 使用されているSystemTap SDT
- ファイル: `tests/include/sdt.h`
- 主要マクロ: `DTRACE_PROBE1`, `DTRACE_PROBE2`, etc.
- 例（usdt_test.c）:
```c
#include "sdt.h"
DTRACE_PROBE2(tracetest, testprobe, tv.tv_sec, "Hello world");
```

### SystemTap SDTの特徴
- **セマフォサポート**: `_SDT_HAS_SEMAPHORES`マクロで制御
- **アーキテクチャサポート**: x86, ARM, PowerPC, s390等に対応
- **アセンブリサポート**: アセンブリコードからも使用可能
- **DTrace互換**: DTRACEマクロも提供
- **制約**: セマフォの使用は全ファイルで統一する必要がある

### 影響を受けるテストプログラム
以下のファイルがsdt.hを使用している：
- `tests/testprogs/usdt_test.c`
- `tests/testprogs/usdt_args.c`
- `tests/testprogs/usdt_inlined.c`
- `tests/testprogs/usdt_lib.c`
- `tests/testprogs/usdt_multi_args.c`
- `tests/testprogs/usdt_multiple_locations.c`
- `tests/testprogs/usdt_quoted_probe.c`
- `tests/testprogs/usdt_semaphore_test.c`
- `tests/testprogs/usdt_sized_args.c`
- `tests/testlibs/usdt_tp.c`

## libbpf/usdtライブラリ

### 特徴
- 単一ヘッダーファイル（usdt.h）
- 依存関係なし
- SystemTap SDTとの100%互換性
- より使いやすいAPI
- セマフォサポートの改善

### 主要API
1. **セマフォなしUSDT**:
   ```c
   USDT(group, name, args...)
   ```

2. **暗黙的セマフォ付きUSDT**:
   ```c
   USDT_WITH_SEMA(group, name, args...)
   USDT_IS_ACTIVE(group, name)
   ```

3. **明示的セマフォ付きUSDT**:
   ```c
   USDT_DEFINE_SEMA(sema)
   USDT_WITH_EXPLICIT_SEMA(sema, group, name, args...)
   USDT_SEMA_IS_ACTIVE(sema)
   ```

### SystemTap SDTとの比較

#### 利点
- **デプロイの簡単さ**: 単一ファイル、依存関係なし
- **セマフォの使いやすさ**: 明示的な定義が不要
- **柔軟性**: セマフォありなしを混在可能
- **共有セマフォ**: 複数のUSDT間でセマフォを共有可能

#### 制限
- **アセンブリサポート**: アセンブリコードからは使用不可（SystemTap SDTは可能）

## 移行計画

### Phase 1: libbpf/usdt.hの導入
1. `tests/include/usdt.h`としてlibbpf/usdt.hをダウンロード
2. 既存のsdt.hと並行して配置

### Phase 2: テストプログラムの変換
各テストプログラムを以下のように変換：

#### 変換例（usdt_test.c）
**変更前**:
```c
#include "sdt.h"
DTRACE_PROBE2(tracetest, testprobe, tv.tv_sec, "Hello world");
```

**変更後**:
```c
#include "usdt.h"
USDT(tracetest, testprobe, tv.tv_sec, "Hello world");
```

### Phase 3: セマフォ使用テストの変換
セマフォを使用するテスト（usdt_semaphore_test.c等）は、新しいAPIに変換：

**変更前**:
```c
// SystemTap SDTのセマフォ使用方法
```

**変更後**:
```c
USDT_WITH_SEMA(tracetest, testprobe, arg1);
if (USDT_IS_ACTIVE(tracetest, testprobe)) {
    // 追加のデータ収集
}
```

### Phase 4: ビルドシステムの更新
- CMakeLists.txtの更新
- 依存関係の調整

### Phase 5: テストの実行と検証
- 既存のランタイムテストがすべて通ることを確認
- 新旧実装の互換性確認

### Phase 6: 古いsdt.hの削除
- tests/include/sdt.hの削除
- 関連する設定ファイルのクリーンアップ

## 次のステップ
1. libbpf/usdt.hのダウンロードと配置
2. 最初のテストプログラム（usdt_test.c）の変換
3. 変換後のテスト実行と動作確認
