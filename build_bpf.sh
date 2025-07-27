#!/bin/bash
set -euxo pipefail

# === 設定 ===
LLVM_VER=17
BPFTRACE_VER=v0.23.5
INSTALL_PREFIX=/usr/local
SRC_DIR="$HOME/code/bpftrace"

# === LLVM 17 のインストール（apt.llvm.org）===
if ! command -v clang-${LLVM_VER} &>/dev/null; then
  echo "[*] Installing LLVM $LLVM_VER ..."
  wget https://apt.llvm.org/llvm.sh
  chmod +x llvm.sh
  sudo ./llvm.sh ${LLVM_VER}
  rm llvm.sh
fi

# === 依存パッケージのインストール ===
echo "[*] Installing build dependencies ..."
sudo apt update
sudo apt install -y \
  git cmake bison flex g++ \
  libelf-dev zlib1g-dev libfl-dev \
  systemtap-sdt-dev binutils-dev libdw-dev \
  libclang-${LLVM_VER}-dev \
  llvm-${LLVM_VER}-dev \
  libedit-dev libxml2-dev

# === ソースコードの取得 ===
echo "[*] Cloning bpftrace $BPFTRACE_VER ..."
rm -rf "$SRC_DIR"
git clone https://github.com/iovisor/bpftrace.git "$SRC_DIR"
cd "$SRC_DIR"
git checkout "$BPFTRACE_VER"
git submodule update --init --recursive

# === 環境変数の設定 ===
export CC=clang-${LLVM_VER}
export CXX=clang++-${LLVM_VER}
export LLVM_DIR="/usr/lib/llvm-${LLVM_VER}"
export PATH="${LLVM_DIR}/bin:$PATH"

# === ビルド ===
echo "[*] Building bpftrace ..."
rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_REQUESTED_VERSION=${LLVM_VER} \
      -DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX} \
      ..
make -j"$(nproc)"

# === インストール ===
echo "[*] Installing bpftrace ..."
sudo make install
sudo ldconfig

# === 動作確認 ===
echo "[*] Done. Version check:"
"${INSTALL_PREFIX}/bin/bpftrace" --version
