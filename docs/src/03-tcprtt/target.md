# 撰寫追蹤程式: 以tcprtt為例

開發 fentry、tracepoint 兩種追蹤程式，輸出 TCP/IPV4 的延遲時間（round trip time）。範例輸出如下

```shell
$ sudo ./tcprtt_tp 
PID     COMM             SRC                         DST                     LAT(ms)
0       swapper/0        192.168.68.64   :53393  --> 20.42.65.94     :47873  196934.87
0       swapper/0        192.168.68.64   :52931  --> 172.217.163.36  :20480  6854.23
```

fentry 使用 ring buffer ， tracepoint 使用 ring buffer 和 hash map，可以參考 bootstrap 使用 map 的流程

## fentry、tracepoint 實作共通說明

- 流程：
    - 核心程式：搜集網路通訊資料，使用 ring buffer 傳遞到用戶
    - 用戶程式：加載和附著核心程式，迴圈不斷接收 ring buffer 資料輸出到終端

- 提供的檔案介紹：
    - **tcprtt.c**：fentry 用戶的完整程式
    - **tcprtt_tp.c**：tracepoint 用戶的完整程式
    - **tcprtt.bpf.c**：待完成的 fentry 核心程式
    - **tcprtt_tp.bpf.c**：待完成的 tracepoint 核心程式

## 完成目標

- 開發 eBPF 核心程式，在用戶輸出 TCP 相關資訊，包含 pid, command, 目標和來源的 ip 及 port 和延遲時間
    1. 完成核心程式 **tcprtt.bpf.c**、**tcprtt_tp.bpf.c**。完成範例程式碼 TODO 部分
    2. 編譯執行檔 **tcprtt**、**tcprtt_tp**
    3. 執行並確認 ip、port
- 比較同一筆連線兩種方式得到的數值是否相符
