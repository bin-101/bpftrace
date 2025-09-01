# PIDネームスペースの概念

## PIDネームスペースとは
PIDネームスペースは、Linuxカーネルの機能の一つで、プロセスIDの空間を分離する仕組みです。コンテナ技術の基盤となる重要な機能です。

## 基本的な仕組み

### 階層構造
- PIDネームスペースは階層構造を持つ
- 親ネームスペースから子ネームスペースを作成可能
- 子ネームスペースのプロセスは、親ネームスペースからも見える

### PIDの見え方
```
初期ネームスペース (PID namespace 0)
├── プロセスA (PID: 1000)
├── プロセスB (PID: 2000) 
└── 子ネームスペース (PID namespace 1)
    ├── プロセスC (PID: 1 in ns1, PID: 3000 in ns0)
    └── プロセスD (PID: 2 in ns1, PID: 4000 in ns0)
```

## コンテナでの利用例

### Dockerコンテナの場合
```bash
# ホスト側から見た場合
$ ps aux | grep nginx
root     12345  nginx: master process

# コンテナ内から見た場合  
$ ps aux | grep nginx
root         1  nginx: master process
```

同じプロセスが：
- ホスト側では PID 12345
- コンテナ内では PID 1

として見える。

## bpftraceへの影響

### 問題のシナリオ
1. **ホストでbpftrace実行 → コンテナ内プロセス監視**
   - bpftraceはグローバルPID（12345）を取得
   - ユーザーはコンテナ内PID（1）を期待
   - 不整合が発生

2. **コンテナ内でbpftrace実行 → 同じコンテナ内プロセス監視**
   - bpftraceはグローバルPID（12345）を取得
   - ユーザーはコンテナ内PID（1）を期待
   - 不整合が発生

### 具体例
```bash
# コンテナ内でbpftraceを実行
$ bpftrace -e 'BEGIN { printf("My PID: %d\n", pid); }'

# PR #3428以前: グローバルPID（例：12345）が表示
# PR #3428以降: ネームスペース内PID（例：1）が表示
```

## eBPFヘルパー関数の動作

### bpf_get_current_pid_tgid()
- 常に初期ネームスペース（グローバル）のPIDを返す
- どのネームスペースから呼び出されても同じ値
- 従来のbpftraceが使用していた方法

### bpf_get_ns_current_pid_tgid()
- 指定されたネームスペース内でのPIDを返す
- そのネームスペース内で実行されているプロセスに対してのみ有効
- ネームスペース外のプロセスに対しては0を返す

## 実際の使用例

### シナリオ1: ホストでbpftrace実行
```bash
# ホスト上でbpftraceを実行
$ bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("PID: %d\n", pid); }'

# コンテナ内のプロセスがファイルを開いた場合
# - bpf_get_current_pid_tgid(): 12345 (グローバルPID)
# - bpf_get_ns_current_pid_tgid(): 12345 (ホストネームスペース内PID)
# 結果: 12345が表示される（正しい）
```

### シナリオ2: コンテナ内でbpftrace実行
```bash
# コンテナ内でbpftraceを実行
$ docker exec -it container bpftrace -e 'tracepoint:syscalls:sys_enter_openat { printf("PID: %d\n", pid); }'

# 同じコンテナ内のプロセスがファイルを開いた場合
# - bpf_get_current_pid_tgid(): 12345 (グローバルPID)
# - bpf_get_ns_current_pid_tgid(): 1 (コンテナ内PID)
# 
# PR #3428以前: 12345が表示される（ユーザーには意味不明）
# PR #3428以降: 1が表示される（ユーザーが期待する値）
```

## PR #3428の解決策
bpftraceが実行されているネームスペースを検出し、適切なヘルパー関数を選択：
- ホストで実行 → `bpf_get_current_pid_tgid()`を使用
- コンテナ内で実行 → `bpf_get_ns_current_pid_tgid()`を使用

これにより、ユーザーが期待するPID値が表示されるようになった。
