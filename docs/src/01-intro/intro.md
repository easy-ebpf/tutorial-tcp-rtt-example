# eBPF 介紹

屬於核心（kernel）程式開發。與核心模組主要差別在**驗證器**（verifier）和 **map** 的設計。驗證器由 Linux kernel 提供，確認被加載的程式的正確性。map 則提供便利的通訊介面，讓 BPF 核心程式間和用戶程式能夠輕易的協作。

## 追蹤程式介紹

eBPF 程式的兩個目的：觀測和改變核心行為，用於觀測的程式可以歸類為追蹤（trace）的類別。

追蹤程式底下可以再分成 kprobe/kretprobe, fentry/fexit 和 tracepoint ：
- kprobe/kretprobe, fentry/fexit: 主要觀察核心中的函式的進入和退出，可以取得呼叫函式的引數（arguments）和回傳值。被追蹤的程式無須修改和重新編譯，屬於動態追蹤的技術。
- tracepoint: 核心程式碼設立和提供 tracepoint 以供觀測，屬於靜態追蹤。這套機制由 Linux 提供，屬於穩定的公開介面。

## 系統架構

系統元件圖:  
![System Block Diagram](https://hackmd.io/_uploads/ByPo075gJe.png)

- `hellp.py`、`hello()`：eBPF 程式，有核心和用戶兩個部分。課程中兩個部分都用 C 開發
- `trace pseudofile`：eBPF 程式核心和用戶溝通的機制，透過特殊檔案或 map
- `execve`、`小蜜蜂`：核心中的 tracepoint ，觸發附著的 eBPF 核心程式
- `Apps`：系統上所有應用程式，可能在系統呼叫時進入 tracepoint 

## 開發工具和設定

- 此篇省略系統設定，例如 Linux Kconfig 的設定
- 開發工具包含函式庫 libbpf （以及相依的 zlib、libelf）和命令列工具 bpftool。

## 檔案介紹

- `<app>.bpf.c` ：核心程式，會轉成 `<app>.skel.h` skeleton 檔供用戶態程式使用
- `<app>.c` ：用戶程式，加載和附著核心程式
- `<app>.h` ：定義核心和用戶溝通時共用的類別
- `vmlinux.h` ：具有 kernel 內所有類別的定義
- `bpftool` ：生成 skeleton 和 `vmlinux.h`
