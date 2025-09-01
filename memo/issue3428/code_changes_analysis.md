# PR #3428 コード変更の詳細分析

## 変更されたファイル一覧
1. `CHANGELOG.md` - 変更ログの更新
2. `src/ast/irbuilderbpf.cpp` - 主要な実装変更
3. `src/ast/irbuilderbpf.h` - ヘッダーファイルの更新
4. `src/ast/passes/codegen_llvm.cpp` - コード生成部分の更新

## 主要な変更内容

### 1. 新しい定数の追加
```cpp
// This constant is defined in the Linux kernel's proc_ns.h
// It represents the inode of the initial (global) PID namespace
constexpr uint32_t PROC_PID_INIT_INO = 0xeffffffc;
```
- 初期PIDネームスペースのinodeを識別するための定数
- この値でbpftraceが初期ネームスペースで実行されているかを判定

### 2. CreateGetPid()関数の大幅な変更

#### 変更前（推測）
```cpp
Value *IRBuilderBPF::CreateGetPid(const location &loc)
{
  Value *pidtgid = CreateGetPidTgid(loc);
  Value *pid = CreateTrunc(CreateLShr(pidtgid, 32), getInt32Ty(), "pid");
  return pid;
}
```

#### 変更後
```cpp
Value *IRBuilderBPF::CreateGetPid(Value *ctx, const location &loc)
{
  const auto &pidns = bpftrace_.get_pidns_self_stat();
  if (pidns.st_ino != PROC_PID_INIT_INO) {
    // Get namespaced target PID when we're running in a namespace
    AllocaInst *res = CreateAllocaBPF(BpfPidnsInfoType(), "bpf_pidns_info");
    CreateGetNsPidTgid(
        ctx, getInt64(pidns.st_dev), getInt64(pidns.st_ino), res, loc);
    Value *pid = CreateLoad(
        getInt32Ty(),
        CreateGEP(BpfPidnsInfoType(), res, { getInt32(0), getInt32(0) }));
    CreateLifetimeEnd(res);
    return pid;
  }

  // Get global target PID when we're in the initial namespace
  Value *pidtgid = CreateGetPidTgid(loc);
  Value *pid = CreateTrunc(CreateLShr(pidtgid, 32), getInt32Ty(), "pid");
  return pid;
}
```

**主要な変更点**:
- `ctx`パラメータの追加
- ネームスペース判定ロジックの追加
- 条件分岐による適切なヘルパー関数の選択

### 3. CreateGetTid()関数の同様の変更
```cpp
Value *IRBuilderBPF::CreateGetTid(Value *ctx, const location &loc)
{
  const auto &pidns = bpftrace_.get_pidns_self_stat();
  if (pidns.st_ino != PROC_PID_INIT_INO) {
    // Get namespaced target TID when we're running in a namespace
    AllocaInst *res = CreateAllocaBPF(BpfPidnsInfoType(), "bpf_pidns_info");
    CreateGetNsPidTgid(
        ctx, getInt64(pidns.st_dev), getInt64(pidns.st_ino), res, loc);
    Value *tid = CreateLoad(
        getInt32Ty(),
        CreateGEP(BpfPidnsInfoType(), res, { getInt32(0), getInt32(1) }));
    CreateLifetimeEnd(res);
    return tid;
  }

  // Get global target TID when we're in the initial namespace
  Value *pidtgid = CreateGetPidTgid(loc);
  Value *tid = CreateTrunc(pidtgid, getInt32Ty(), "tid");
  return tid;
}
```

### 4. 新しいヘルパー関数の追加

#### CreateGetNsPidTgid()
```cpp
void IRBuilderBPF::CreateGetNsPidTgid(Value *ctx,
                                      Value *dev,
                                      Value *ino,
                                      AllocaInst *ret,
                                      const location &loc)
{
  // long bpf_get_ns_current_pid_tgid(
  //   u64 dev, u64 ino, struct bpf_pidns_info *nsdata, u32 size)
  // Return: 0 on success
  auto &layout = module_.getDataLayout();
  auto struct_size = layout.getTypeAllocSize(BpfPidnsInfoType());

  FunctionType *getnspidtgid_func_type = FunctionType::get(
      getInt64Ty(),
      {
          getInt64Ty(),
          getInt64Ty(),
          BpfPidnsInfoType()->getPointerTo(),
          getInt32Ty(),
      },
      false);
  CallInst *call = CreateHelperCall(libbpf::BPF_FUNC_get_ns_current_pid_tgid,
                                    getnspidtgid_func_type,
                                    { dev, ino, ret, getInt32(struct_size) },
                                    "get_ns_pid_tgid",
                                    &loc);
  CreateHelperErrorCond(
      ctx, call, libbpf::BPF_FUNC_get_ns_current_pid_tgid, loc);
}
```

#### BpfPidnsInfoType()
```cpp
llvm::Type *IRBuilderBPF::BpfPidnsInfoType()
{
  return GetStructType("bpf_pidns_info",
                       {
                           getInt32Ty(),  // PID
                           getInt32Ty(),  // TID
                       },
                       false);
}
```

### 5. 関数シグネチャの変更
多くの関数で`ctx`パラメータが追加された：
- `CreateGetPid(ctx, loc)`
- `CreateGetTid(ctx, loc)`
- `CreateUSym(ctx, val, probe_id, loc)`

## 実装の仕組み

### 1. ネームスペース判定
```cpp
const auto &pidns = bpftrace_.get_pidns_self_stat();
if (pidns.st_ino != PROC_PID_INIT_INO) {
    // ネームスペース内で実行中
} else {
    // 初期ネームスペースで実行中
}
```

### 2. 適切なヘルパー関数の選択
- **初期ネームスペース**: `bpf_get_current_pid_tgid()`を使用
- **子ネームスペース**: `bpf_get_ns_current_pid_tgid()`を使用

### 3. bpf_pidns_info構造体
```c
struct bpf_pidns_info {
    u32 pid;  // ネームスペース内のPID
    u32 tid;  // ネームスペース内のTID
};
```

## 影響範囲
- `pid`ビルトイン
- `tid`ビルトイン
- `ustack`ビルトイン（PIDが必要なため）
- `usym()`関数（内部でPIDを使用）

## 次の調査項目
1. `bpftrace_.get_pidns_self_stat()`の実装
2. テストケースの確認
3. 実際の動作確認方法
