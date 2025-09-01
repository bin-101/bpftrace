# PIDネームスペース検出の実装詳細

## get_pidns_self_stat()の実装

### 実装場所
`src/bpftrace.cpp` の最後の方に実装されている。

### 実装コード
```cpp
const std::optional<struct stat> &BPFtrace::get_pidns_self_stat() const
{
  static std::optional<struct stat> pidns = []() -> std::optional<struct stat> {
    struct stat s;
    if (::stat("/proc/self/ns/pid", &s)) {
      if (errno == ENOENT)
        return std::nullopt;
      throw std::runtime_error(
          std::string("Failed to stat /proc/self/ns/pid: ") +
          std::strerror(errno));
    }
    return s;
  }();

  return pidns;
}
```

## 実装の詳細分析

### 1. 静的初期化
```cpp
static std::optional<struct stat> pidns = []() -> std::optional<struct stat> {
    // 初期化処理
}();
```
- **静的変数**: プログラム実行中に一度だけ初期化される
- **ラムダ式による初期化**: 複雑な初期化ロジックを実行
- **std::optional**: ネームスペースが存在しない場合を表現

### 2. /proc/self/ns/pidの利用
```cpp
if (::stat("/proc/self/ns/pid", &s)) {
```
- **`/proc/self/ns/pid`**: 現在のプロセスのPIDネームスペースを表すシンボリックリンク
- **`stat()`システムコール**: ファイルの情報（inode番号など）を取得
- **`::stat`**: グローバルスコープのstat関数を明示的に呼び出し

### 3. エラーハンドリング
```cpp
if (errno == ENOENT)
  return std::nullopt;
throw std::runtime_error(
    std::string("Failed to stat /proc/self/ns/pid: ") +
    std::strerror(errno));
```
- **ENOENT**: ファイルが存在しない（古いカーネルなど）
- **その他のエラー**: 例外を投げる

## /proc/self/ns/pidの仕組み

### 通常の状態
```bash
$ ls -la /proc/self/ns/pid
lrwxrwxrwx 1 user user 0 Jan  1 12:00 /proc/self/ns/pid -> 'pid:[4026531836]'

$ stat /proc/self/ns/pid
  File: /proc/self/ns/pid -> 'pid:[4026531836]'
  Size: 0               Blocks: 0          IO Block: 1024   symbolic link
  Device: 4h/4d   Inode: 4026531836  Links: 1
```

### コンテナ内の状態
```bash
# コンテナ内
$ ls -la /proc/self/ns/pid
lrwxrwxrwx 1 root root 0 Jan  1 12:00 /proc/self/ns/pid -> 'pid:[4026532448]'

$ stat /proc/self/ns/pid
  File: /proc/self/ns/pid -> 'pid:[4026532448]'
  Size: 0               Blocks: 0          IO Block: 1024   symbolic link
  Device: 4h/4d   Inode: 4026532448  Links: 1
```

## ネームスペース判定の仕組み

### 定数との比較
```cpp
// src/ast/irbuilderbpf.cpp
constexpr uint32_t PROC_PID_INIT_INO = 0xeffffffc;

const auto &pidns = bpftrace_.get_pidns_self_stat();
if (pidns.st_ino != PROC_PID_INIT_INO) {
    // ネームスペース内で実行中
} else {
    // 初期ネームスペースで実行中
}
```

### PROC_PID_INIT_INOの意味
- **値**: `0xeffffffc` (4026531836 in decimal)
- **意味**: 初期PIDネームスペースのinode番号
- **定義場所**: Linuxカーネルの`include/linux/proc_ns.h`

### 判定ロジック
1. `/proc/self/ns/pid`をstat()でinode番号を取得
2. inode番号が`PROC_PID_INIT_INO`と等しいかチェック
3. 等しい → 初期ネームスペース
4. 異なる → 子ネームスペース

## 実際の使用例

### ホスト環境
```cpp
// get_pidns_self_stat()が返すstat構造体
struct stat {
    ...
    ino_t st_ino = 4026531836;  // PROC_PID_INIT_INO と同じ
    ...
}

// 判定結果
pidns.st_ino == PROC_PID_INIT_INO  // true
// → bpf_get_current_pid_tgid()を使用
```

### コンテナ環境
```cpp
// get_pidns_self_stat()が返すstat構造体
struct stat {
    ...
    ino_t st_ino = 4026532448;  // PROC_PID_INIT_INO と異なる
    ...
}

// 判定結果
pidns.st_ino != PROC_PID_INIT_INO  // true
// → bpf_get_ns_current_pid_tgid()を使用
```

## パフォーマンス考慮

### 静的初期化の利点
- プログラム開始時に一度だけ実行
- 毎回のPID/TID取得時にstat()を呼ぶ必要がない
- ネームスペースは実行中に変わらないため、キャッシュが有効

### メモリ効率
- `std::optional<struct stat>`のサイズは小さい
- 静的変数なので追加のメモリ割り当てなし

## エラーケース

### 古いカーネル
- `/proc/self/ns/pid`が存在しない
- `std::nullopt`を返す
- 呼び出し側でnullチェックが必要

### 権限エラー
- 通常は発生しないが、特殊な環境では可能
- 例外を投げて処理を停止

## まとめ
この実装は、Linuxのプロセスネームスペース機能を活用して、bpftraceが実行されているネームスペースを効率的に判定する仕組みです。静的初期化により高いパフォーマンスを実現し、適切なエラーハンドリングも含まれています。
