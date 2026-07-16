#!/usr/bin/env python3
"""
MicroPython QSTR 生成脚本 (for e2studio/GCC RA8P1 project)

替代 MicroPython Makefile 构建系统中的 qstr 生成流程。
从 SOURCE_GROUPS 的 scan_dirs 递归发现 .c 文件，自动完成预处理和 qstr 注册。

用法:
  python gen_qstr.py                    # 生成所有 genhdr 文件
  python gen_qstr.py --dry-run          # 预览操作
  python gen_qstr.py --verbose          # 详细输出
  python gen_qstr.py --gcc-path PATH    # 指定 arm-none-eabi-gcc 路径
"""

import os
import re
import sys
import shutil
import subprocess
import argparse
from pathlib import Path


# ╔══════════════════════════════════════════════════════════╗
# ║                    项目路径常量                            ║
# ╚══════════════════════════════════════════════════════════╝

PROJECT_DIR = Path(__file__).resolve().parent
# e2studio project root (parent of src/MicroPython) — FSP/RA headers live here
PROJECT_ROOT = PROJECT_DIR.parent.parent
PY_DIR = PROJECT_DIR / "py"
GENHDR_DIR = PROJECT_DIR / "genhdr"
PORT_DIR = PROJECT_DIR / "ports" / "renesas-ra"

MAKEQSTRDEFS_PY = PY_DIR / "makeqstrdefs.py"
MAKEQSTRDATA_PY = PY_DIR / "makeqstrdata.py"
MAKEVERSIONHDR_PY = PY_DIR / "makeversionhdr.py"
MAKEMODULEDEFS_PY = PY_DIR / "makemoduledefs.py"
MAKECOMPRESSEDDATA_PY = PY_DIR / "makecompresseddata.py"
MAKE_ROOT_POINTERS_PY = PY_DIR / "make_root_pointers.py"

QSTRDEFS_PY = PY_DIR / "qstrdefs.h"
QSTRDEFS_PORT = PORT_DIR / "qstrdefsport.h"
QSTRDEFS_GENERATED = GENHDR_DIR / "qstrdefs.generated.h"


# ╔══════════════════════════════════════════════════════════╗
# ║              源文件分组配置（可按需增减）                    ║
# ║                                                            ║
# ║  每个组包含:                                                ║
# ║    enabled   — 是否启用                                     ║
# ║    label     — 显示名称                                     ║
# ║    scan_dirs — 递归扫描目录，发现 .c 文件参与 qstr 预处理    ║
# ║    extra_files — 额外 .c 文件（不在 scan_dirs 范围内的）     ║
# ╚══════════════════════════════════════════════════════════╝

SOURCE_GROUPS = [
    # ──────────── MicroPython py/ 核心 ────────────
    {
        "enabled": True,
        "label": "MicroPython py/ 核心",
        "scan_dirs": ["py"],
        "extra_files": [],
    },

    # ──────────── extmod/ 扩展模块 ────────────
    {
        "enabled": True,
        "label": "extmod/ 扩展模块",
        "scan_dirs": ["extmod"],
        "extra_files": [],
    },

    # ──────────── ports/renesas-ra/ 端口层 ────────────
    {
        "enabled": True,
        "label": "ports/renesas-ra/ 端口层",
        "scan_dirs": ["ports/renesas-ra"],
        "extra_files": [],
    },

    # ──────────── shared/ 共享代码 ────────────
    {
        "enabled": True,
        "label": "shared/ 共享代码",
        "scan_dirs": ["shared"],
        "extra_files": [],
    },

    # ──────────── OpenMV 机器视觉 ────────────
    {
        "enabled": False,
        "label": "OpenMV 机器视觉",
        "scan_dirs": [
            "OpenMV/src/omv",
            "OpenMV/src/omv/modules",
            "OpenMV/src/omv/imlib",
            "OpenMV/src/omv/ports/ra8",
            "OpenMV/src/omv/common",
        ],
        "extra_files": [],
    },

    # ──────────── src/ 应用入口 ────────────
    {
        "enabled": False,
        "label": "user/ 应用入口",
        "scan_dirs": ["user"],
        "extra_files": [],
    },

    # ──────────── drivers/ 驱动 ────────────
    {
        "enabled": True,
        "label": "drivers/ 驱动",
        "scan_dirs": ["drivers"],
        "extra_files": [],
    },
]


# ╔══════════════════════════════════════════════════════════╗
# ║                  编译器标志配置                             ║
# ╚══════════════════════════════════════════════════════════╝

# Paths relative to PROJECT_ROOT (e2studio project root — FSP/RA headers)
ROOT_INCLUDE_PATHS = [
    "ra_gen", ".", "ra_cfg/fsp_cfg/bsp", "ra_cfg/fsp_cfg",
    "ra/arm/CMSIS_6/CMSIS/Core/Include", "src",
    "ra/fsp/inc", "ra/fsp/inc/api", "ra/fsp/inc/instances",
    "Debug",
]

# Paths relative to PROJECT_DIR (src/MicroPython — MicroPython headers)
MP_INCLUDE_PATHS = [
    "genhdr", "genhdr/compress", "genhdr/qstr", "genhdr/module", "genhdr/root_pointer",
    "py", "shared", "ports/renesas-ra",
]

COMPILE_DEFINES = [
    "_RENESAS_RA_", "_RA_CORE=CPU0", "_RA_ORDINAL=1",
    "STATIC=static", "BSP_MULTICORE_PROJECT=1", "ARM_MATH_CM85",
]

FORCE_INCLUDE = "ports/renesas-ra/mpconfigport.h"

GCC_FALLBACK_PATHS = [
    "D:/Single-chip/MySoftware/e2studiofsp6.4/toolchains/gcc_arm/13.2.rel1/bin/arm-none-eabi-gcc.exe",
]


# ╔══════════════════════════════════════════════════════════╗
# ║                  工具函数                                  ║
# ╚══════════════════════════════════════════════════════════╝

def collect_source_files():
    """从 SOURCE_GROUPS 递归发现所有 .c 文件"""
    seen = set()
    result = []

    for group in SOURCE_GROUPS:
        if not group["enabled"]:
            continue

        # 从 scan_dirs 递归发现 .c 文件
        for d in group["scan_dirs"]:
            scan_dir = PROJECT_DIR / d
            if not scan_dir.exists():
                continue
            for root, dirs, files in os.walk(scan_dir):
                # 跳过 genhdr、build 目录
                dirs[:] = [x for x in dirs if x not in ("genhdr", "build", "Debug")]
                for fname in files:
                    if not fname.endswith(".c"):
                        continue
                    fpath = (Path(root) / fname).resolve()
                    if fpath not in seen:
                        seen.add(fpath)
                        result.append(str(fpath))

        # 额外指定的文件
        for src in group.get("extra_files", []):
            fpath = (PROJECT_DIR / src).resolve()
            if fpath.exists() and fpath not in seen:
                seen.add(fpath)
                result.append(str(fpath))

    return result


def load_compile_commands():
    """从 compile_commands.json 读取 GCC 路径、-I 和 -D 标志"""
    cc_json = PROJECT_ROOT / "Debug" / "compile_commands.json"
    if not cc_json.exists():
        return None
    import json
    try:
        with open(cc_json, "r", encoding="utf-8") as f:
            entries = json.load(f)
        if not entries:
            return None
        cmd = entries[0].get("command", "")
        # Extract GCC path (first token, possibly quoted)
        gcc_path = None
        rest = cmd
        if rest.startswith('"'):
            end = rest.index('"', 1)
            gcc_path = rest[1:end]
            rest = rest[end + 1:]
        else:
            parts = rest.split(None, 1)
            gcc_path = parts[0]
            rest = parts[1] if len(parts) > 1 else ""
        # Extract -I and -D from remaining command
        import shlex
        tokens = shlex.split(rest)
        includes = []
        defines = []
        i = 0
        while i < len(tokens):
            t = tokens[i]
            if t.startswith("-I"):
                path = t[2:] or (tokens[i + 1] if i + 1 < len(tokens) else "")
                if path:
                    includes.append(path)
                if not t[2:]:
                    i += 1
            elif t.startswith("-D"):
                val = t[2:] or (tokens[i + 1] if i + 1 < len(tokens) else "")
                if val:
                    defines.append(val)
                if not t[2:]:
                    i += 1
            i += 1
        return {"gcc_path": gcc_path, "includes": includes, "defines": defines}
    except Exception:
        return None


def find_gcc_path(cc_info=None):
    """自动检测 arm-none-eabi-gcc 路径"""
    if cc_info and cc_info.get("gcc_path"):
        return cc_info["gcc_path"]
    for p in GCC_FALLBACK_PATHS:
        if Path(p).exists():
            return p
    gcc = shutil.which("arm-none-eabi-gcc")
    return gcc


def get_include_flags(cc_info=None):
    """构建 -I 标志列表。优先使用 compile_commands.json，否则回退到硬编码路径。"""
    if cc_info and cc_info.get("includes"):
        # Use includes from compile_commands.json, resolving relative paths
        flags = []
        for p in cc_info["includes"]:
            # Resolve "." to PROJECT_ROOT (e2studio runs compiler from project root)
            if p == ".":
                flags.append(f"-I{PROJECT_ROOT}")
            elif Path(p).exists():
                flags.append(f"-I{p}")
        # Add Debug/ dir (contains bsp_linker_info.h etc.)
        debug_dir = PROJECT_ROOT / "Debug"
        if debug_dir.exists() and f"-I{debug_dir}" not in flags:
            flags.append(f"-I{debug_dir}")
        # Add MicroPython genhdr sub-dirs not present in e2studio build
        for p in MP_INCLUDE_PATHS:
            d = PROJECT_DIR / p
            if d.exists() and f"-I{d}" not in flags:
                flags.append(f"-I{d}")
        return flags
    # Fallback: FSP/RA headers from project root
    flags = []
    for p in ROOT_INCLUDE_PATHS:
        d = PROJECT_ROOT / p
        if d.exists():
            flags.append(f"-I{d}")
    # MicroPython headers from MicroPython dir
    for p in MP_INCLUDE_PATHS:
        d = PROJECT_DIR / p
        if d.exists():
            flags.append(f"-I{d}")
    return flags


def get_define_flags(cc_info=None):
    """构建 -D 标志列表。优先使用 compile_commands.json，否则回退到硬编码定义。"""
    if cc_info and cc_info.get("defines"):
        return [f"-D{d}" for d in cc_info["defines"]]
    return [f"-D{d}" for d in COMPILE_DEFINES]


def get_cflags(gcc_path, no_qstr=True, cc_info=None):
    flags = [
        gcc_path, "-mcpu=cortex-m85", "-mthumb", "-mlittle-endian",
        "-mfloat-abi=hard", "-mfpu=fpv5-d16", "-E",
        "-Os", "-fdata-sections", "-ffunction-sections",
        "-fno-strict-aliasing", "-fshort-enums", "-fno-unroll-loops",
        "-fsigned-char", "-w", "-std=c99",
    ]
    flags += get_include_flags(cc_info)
    flags += get_define_flags(cc_info)
    if no_qstr:
        flags.append("-DNO_QSTR")
    # Force include mpconfigport.h before any source file
    force_include = PROJECT_DIR / FORCE_INCLUDE
    if force_include.exists():
        flags += [f"-include{force_include}"]
    return flags


def run_cmd(cmd, verbose=False, dry_run=False):
    if dry_run or verbose:
        print(f"  CMD: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    if dry_run:
        return b""
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        stderr = result.stderr.decode("utf-8", errors="replace")
        print(f"  WARNING: command returned {result.returncode}")
        if stderr:
            lines = stderr.strip().split("\n")
            for line in lines[:5]:
                print(f"    {line}")
            if len(lines) > 5:
                print(f"    ... ({len(lines) - 5} more lines)")
    return result.stdout


# ╔══════════════════════════════════════════════════════════╗
# ║                  qstr 生成流程                              ║
# ╚══════════════════════════════════════════════════════════╝

def step1_preprocess(gcc_path, sources, verbose, dry_run, cc_info=None):
    """Step 1: 对所有源文件运行 C 预处理器"""
    print("\n[Step 1] 预处理所有源文件...")
    output_file = GENHDR_DIR / "qstr.i.last"
    cflags = get_cflags(gcc_path, no_qstr=True, cc_info=cc_info)
    if dry_run:
        print(f"  输出: {output_file}, 源文件数: {len(sources)}")
        return output_file
    all_output = b""
    failed = 0
    for i, src in enumerate(sources):
        if verbose:
            print(f"  [{i+1}/{len(sources)}] {os.path.relpath(src, PROJECT_DIR)}")
        output = run_cmd(cflags + [src], verbose=verbose, dry_run=dry_run)
        if output:
            all_output += output
        else:
            failed += 1
    with open(output_file, "wb") as f:
        f.write(all_output)
    print(f"  完成: {len(sources)} 个文件, {failed} 个失败")
    return output_file


def step2_split(qstr_last, verbose, dry_run):
    """Step 2: 分割预处理输出"""
    print("\n[Step 2] 分割预处理输出...")
    for mode in ["qstr", "compress", "module", "root_pointer"]:
        output_dir = GENHDR_DIR / mode
        if not dry_run:
            if output_dir.exists():
                for old_file in output_dir.glob(f"*.{mode}"):
                    old_file.unlink()
            output_dir.mkdir(parents=True, exist_ok=True)
        run_cmd([
            sys.executable, str(MAKEQSTRDEFS_PY), "split", mode,
            str(qstr_last), str(output_dir), "_",
        ], verbose=verbose, dry_run=dry_run)
    print("  完成")


def step3_cat(verbose, dry_run):
    """Step 3: 合并分割文件"""
    print("\n[Step 3] 合并分割文件...")
    for mode, output_name in [
        ("qstr", "qstrdefs.collected.h"), ("compress", "compressed.collected"),
        ("module", "moduledefs.collected"), ("root_pointer", "root_pointers.collected"),
    ]:
        run_cmd([
            sys.executable, str(MAKEQSTRDEFS_PY), "cat", mode, "_",
            str(GENHDR_DIR / mode), str(GENHDR_DIR / output_name),
        ], verbose=verbose, dry_run=dry_run)
    print("  完成")


def step4_preprocess_qstr(gcc_path, verbose, dry_run, cc_info=None):
    """Step 4: 二次预处理 qstr 定义"""
    print("\n[Step 4] 二次预处理 qstr 定义...")
    collected = GENHDR_DIR / "qstrdefs.collected.h"
    preprocessed = GENHDR_DIR / "qstrdefs.preprocessed.h"
    if dry_run:
        print(f"  输出: {preprocessed}")
        return
    with open(QSTRDEFS_PY, "r", encoding="utf-8", errors="replace") as f:
        content = f.read()
    with open(QSTRDEFS_PORT, "r", encoding="utf-8", errors="replace") as f:
        content += "\n" + f.read()
    with open(collected, "r", encoding="utf-8", errors="replace") as f:
        content += "\n" + f.read()
    # QCFG 不包裹引号（需宏展开），Q(...) 包裹引号
    wrapped_lines = []
    for line in content.split("\n"):
        s = line.strip()
        if s.startswith("QCFG("):
            wrapped_lines.append(s)
        elif s.startswith("Q(") and s.endswith(")"):
            wrapped_lines.append(f'"{s}"')
        else:
            wrapped_lines.append(line)
    wrapped = "\n".join(wrapped_lines)
    cflags = get_cflags(gcc_path, no_qstr=False, cc_info=cc_info)
    result = subprocess.run(cflags + ["-x", "c", "-"], input=wrapped.encode("utf-8"), capture_output=True)
    if result.returncode != 0:
        stderr = result.stderr.decode("utf-8", errors="replace")
        print(f"  WARNING: 预处理返回 {result.returncode}")
        if stderr:
            for line in stderr.strip().split("\n")[:5]:
                print(f"    {line}")
    pp_output = result.stdout.decode("utf-8", errors="replace")
    # Normalize line endings (GCC on Windows may output \r\n)
    pp_output = pp_output.replace("\r\n", "\n").replace("\r", "\n")
    unwrapped = re.sub(r'^"?(Q\(.*?\))"?$', r'\1', pp_output, flags=re.MULTILINE)
    unwrapped = re.sub(r'^"?(QCFG\(.*?\))"?$', r'\1', unwrapped, flags=re.MULTILINE)
    with open(preprocessed, "w", encoding="utf-8") as f:
        f.write(unwrapped)
    print(f"  输出: {preprocessed}")


def step5_generate_qstr(verbose, dry_run):
    """Step 5: 生成 qstrdefs.generated.h"""
    print("\n[Step 5] 生成 qstrdefs.generated.h...")
    preprocessed = GENHDR_DIR / "qstrdefs.preprocessed.h"
    generated = GENHDR_DIR / "qstrdefs.generated.h"
    if dry_run:
        print(f"  输出: {generated}")
        return
    result = subprocess.run([sys.executable, str(MAKEQSTRDATA_PY), str(preprocessed)], capture_output=True)
    if result.returncode != 0:
        print(f"  ERROR: {result.stderr.decode('utf-8', errors='replace')}")
        return
    with open(generated, "wb") as f:
        f.write(result.stdout)
    print(f"  输出: {generated}")


def step6_generate_derived(verbose, dry_run):
    """生成 moduledefs.h / root_pointers.h / compressed.data.h / mpversion.h"""
    derived = [
        ("moduledefs.h", MAKEMODULEDEFS_PY, "moduledefs.collected"),
        ("root_pointers.h", MAKE_ROOT_POINTERS_PY, "root_pointers.collected"),
        ("compressed.data.h", MAKECOMPRESSEDDATA_PY, "compressed.collected"),
    ]
    for output_name, script, input_name in derived:
        print(f"\n[Step 6] 生成 {output_name}...")
        if dry_run:
            print(f"  输出: {GENHDR_DIR / output_name}")
            continue
        result = subprocess.run([sys.executable, str(script), str(GENHDR_DIR / input_name)], capture_output=True)
        if result.returncode != 0:
            print(f"  ERROR: {result.stderr.decode('utf-8', errors='replace')}")
            continue
        with open(GENHDR_DIR / output_name, "wb") as f:
            f.write(result.stdout)
        print(f"  输出: {GENHDR_DIR / output_name}")

    # mpversion.h
    print("\n[Step 7] 生成 mpversion.h...")
    if not dry_run:
        run_cmd([sys.executable, str(MAKEVERSIONHDR_PY), str(GENHDR_DIR / "mpversion.h")], verbose=verbose)
        print(f"  输出: {GENHDR_DIR / 'mpversion.h'}")

    # backup
    print("\n[Step 8] 更新 qstrdefs.generated.backup.h...")
    generated = GENHDR_DIR / "qstrdefs.generated.h"
    backup = GENHDR_DIR / "qstrdefs.generated.backup.h"
    if not dry_run and generated.exists():
        shutil.copy2(generated, backup)
        print(f"  输出: {backup}")


# ╔══════════════════════════════════════════════════════════╗
# ║                    主入口                                   ║
# ╚══════════════════════════════════════════════════════════╝

def main():
    parser = argparse.ArgumentParser(
        description="MicroPython QSTR 生成脚本 (e2studio/GCC RA8P1)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python gen_qstr.py                  # 完整生成
  python gen_qstr.py --dry-run        # 预览
  python gen_qstr.py --verbose        # 详细输出
  python gen_qstr.py --gcc-path PATH  # 指定 arm-none-eabi-gcc
        """,
    )
    parser.add_argument("--dry-run", action="store_true", help="只打印操作，不执行")
    parser.add_argument("--verbose", "-v", action="store_true", help="详细输出")
    parser.add_argument("--gcc-path", type=str, default=None, help="arm-none-eabi-gcc 可执行文件路径")
    args = parser.parse_args()

    print("=" * 60)
    print("MicroPython QSTR 生成")
    print(f"项目目录: {PROJECT_DIR}")
    print("=" * 60)

    # 显示启用的源组
    for group in SOURCE_GROUPS:
        status = "ON" if group["enabled"] else "OFF"
        n_dirs = len(group["scan_dirs"])
        n_extra = len(group.get("extra_files", []))
        print(f"  [{status}] {group['label']} ({n_dirs} dirs, {n_extra} extra files)")

    # 从 compile_commands.json 加载项目编译信息
    cc_info = load_compile_commands()
    if cc_info:
        print(f"\n  从 compile_commands.json 读取到:")
        print(f"    GCC: {cc_info['gcc_path']}")
        print(f"    -I 路径: {len(cc_info['includes'])} 个")
        print(f"    -D 定义: {len(cc_info['defines'])} 个")
    else:
        print("\n  未找到 compile_commands.json，使用硬编码路径")

    gcc_path = args.gcc_path or find_gcc_path(cc_info)
    if not gcc_path:
        print("\nERROR: 找不到 arm-none-eabi-gcc！请使用 --gcc-path 指定路径")
        sys.exit(1)
    print(f"\nGCC: {gcc_path}")

    if not args.dry_run:
        GENHDR_DIR.mkdir(parents=True, exist_ok=True)
        for sub in ["qstr", "compress", "module", "root_pointer"]:
            (GENHDR_DIR / sub).mkdir(parents=True, exist_ok=True)

    sources = collect_source_files()
    print(f"源文件数: {len(sources)}")
    if not sources:
        print("ERROR: 没有找到源文件！")
        sys.exit(1)

    qstr_last = step1_preprocess(gcc_path, sources, args.verbose, args.dry_run, cc_info)
    step2_split(qstr_last, args.verbose, args.dry_run)
    step3_cat(args.verbose, args.dry_run)
    step4_preprocess_qstr(gcc_path, args.verbose, args.dry_run, cc_info)
    step5_generate_qstr(args.verbose, args.dry_run)
    step6_generate_derived(args.verbose, args.dry_run)

    print("\n" + "=" * 60)
    print("QSTR 生成完成！")
    for name in [
        "mpversion.h", "qstrdefs.collected.h", "qstrdefs.preprocessed.h",
        "qstrdefs.generated.h", "qstrdefs.generated.backup.h",
        "moduledefs.h", "root_pointers.h", "compressed.data.h",
    ]:
        f = GENHDR_DIR / name
        if f.exists():
            print(f"  {name:40s} {f.stat().st_size:>10,} bytes")
        else:
            print(f"  {name:40s}    (未生成)")
    print("=" * 60)


if __name__ == "__main__":
    main()
