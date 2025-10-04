#!/usr/bin/env python3
"""tests/runtime/engine/runner.py を理解するためのコメント付き参考コード。
   実際のテスト実行には使われません。"""

import subprocess
import signal
import os

class Runner:
    def run_case(self, case, bpftrace_path):
        prog = case["PROG"]
        proc = subprocess.Popen([bpftrace_path, "-e", prog], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        output, _ = proc.communicate()

        expected = case.get("EXPECT")
        if expected:
            if expected not in output:
                raise AssertionError("EXPECTED text not found")

        if case.get("WILL_FAIL"):
            if proc.returncode == 0:
                raise AssertionError("Expected failure but got success")
        else:
            if proc.returncode != 0:
                raise AssertionError("Expected success but command failed")

        print(f"Case {case['NAME']} passed")

# 実際の runner.py では複数のケースを読み込み、シグナル制御や
# タイムアウト、環境変数の扱いなどさらに詳細な処理を行っています。
