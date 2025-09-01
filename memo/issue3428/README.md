# Issue #4253 & PR #3428 調査資料

## 推奨読書順序

この資料は段階的に理解を深められるよう構成されています。以下の順番で読むことをお勧めします：

### 1. 基礎理解フェーズ

#### 📖 `pr3428_overview.md`
**最初に読むべきファイル**
- PR #3428の基本情報と問題の概要
- 何が修正されたかの全体像
- 読了時間: 約5分

#### 📖 `pid_namespace_concept.md`
**PIDネームスペースの基礎知識**
- Linuxのネームスペース機能の説明
- コンテナでの動作例
- bpftraceへの影響
- 読了時間: 約10分

### 2. 技術詳細フェーズ

#### 📖 `code_changes_analysis.md`
**実装の詳細**
- 具体的なコード変更内容
- 関数の変更前後の比較
- 新しく追加された機能
- 読了時間: 約15分

#### 📖 `pidns_detection_implementation.md`
**ネームスペース検出の仕組み**
- `get_pidns_self_stat()`の実装詳細
- `/proc/self/ns/pid`の活用方法
- パフォーマンス考慮事項
- 読了時間: 約10分

### 3. 統合理解フェーズ

#### 📖 `summary_and_issue4253_connection.md`
**全体まとめと今後の課題**
- PR #3428の成果と限界
- Issue #4253で提起された新たな問題
- 提案されている解決策
- 読了時間: 約15分

## 読書のコツ

### 初回読書時
1. **概要から詳細へ**: まず全体像を把握してから詳細に入る
2. **実例重視**: コード例やシナリオを重点的に理解する
3. **疑問点メモ**: 理解できない部分は後で調べるためメモしておく

### 復習時
- `summary_and_issue4253_connection.md`から読み始めて全体を思い出す
- 必要に応じて個別のファイルを参照

## 各ファイルの特徴

| ファイル | 難易度 | 重要度 | 内容の性質 |
|---------|--------|--------|------------|
| `pr3428_overview.md` | ⭐ | ⭐⭐⭐ | 概要・導入 |
| `pid_namespace_concept.md` | ⭐⭐ | ⭐⭐⭐ | 基礎知識 |
| `code_changes_analysis.md` | ⭐⭐⭐ | ⭐⭐ | 技術詳細 |
| `pidns_detection_implementation.md` | ⭐⭐⭐ | ⭐⭐ | 実装詳細 |
| `summary_and_issue4253_connection.md` | ⭐⭐ | ⭐⭐⭐ | 統合・展望 |

## 前提知識

### 必須
- Linuxの基本的なプロセス概念
- コンテナ（Docker）の基本的な使用経験

### あると良い
- eBPF/bpftraceの基本知識
- C++プログラミングの経験
- システムプログラミングの知識

## 関連リンク

- [GitHub PR #3428](https://github.com/bpftrace/bpftrace/pull/3428)
- [GitHub Issue #4253](https://github.com/bpftrace/bpftrace/issues/4253)
- [Linux Namespaces Documentation](https://man7.org/linux/man-pages/man7/namespaces.7.html)

## 質問・疑問がある場合

各ファイルを読んで疑問が生じた場合は、以下の順序で解決を試してください：

1. **他のファイルを参照**: 関連する説明が別のファイルにある可能性
2. **実際のコードを確認**: `src/`ディレクトリ内の実装を直接確認
3. **GitHubの議論を参照**: PR/Issueのコメント欄に詳細な議論がある場合

---

**最終更新**: 2025年9月1日  
**作成者**: 調査メモ
