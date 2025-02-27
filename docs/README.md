# mdBook使用說明

- 執行指令請先cd至本目錄。

## 0. 設定mdBook:
```shell
# macOS
brew install mdbook

# cargo
cargo install mdbook
```

- 完成後可以直接開啟mdbook進行預覽:
```shell
mdbook serve
```


## 1. 目錄檔: `src/SUMMARY.md`

## 2. 在目錄內連接指定的markdown檔案
- 如果你有開`mdbook serve`，你如果連了不存在的檔案，它會幫你建。

## 3. 如果要生成網頁，就使用`mdbook build`。
