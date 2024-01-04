# cc for dmc

gcc/cc風のコマンドライン引数で dmc でコンパイルするためのラッパー。  
オプション文字列の変換とファイルパスの '/' を '\\' に変換する。

cmake ＆ gnu make を使って dmc でのコンパイルを試しみるために用意。

## 利用方法

- bin\cc.exe を dmc.exe のあるフォルダに入れる
- 環境変数 DMC_DIR に dmc ディレクトリを設定しておく
- dmc ディレクトリを c:\dmc か c:\dm にする

のいずれかを行う。

## ビルド

https://github.com/digitalmars/dmc.git

を clone した dmc 環境を使う。  
※ 付属の stlport を使うため、この環境が楽。

dmc\bin へのパスが通った状態で、bld\mk.bat を実行。

## オプション

```
  --help    help.  
  --DMC     Afterwards dmc option.  
  --GCC     Afterwards gcc option.  
 (gcc)                    (dmc)  
  --define-macro M[=S]    -D[M[=S]]  
  -D MACRO[=STR]          -D[MACRO[=STR]]  
  --undefine-macro MACRO  -U[MACRO]  
  -U MACRO                -U[MACRO]  
  --include-directory DIR -I[DIR]  
  -I DIR                  -I[DIR]  
  --output FILE           -o[FILE]  
  -o FILE                 -o[FILE]  
  -S                      -cod  
  -shared                 -WD  
  -mdll                   -WD  
  --debug                 -g  
  -g                      -g  
  -Wall                   -w  
  -Werror                 -wx  
  -O0                     -o+none  
  -O1 -O2 -O3             -o+all  
  -Ofast                  -o+speed  
  -Os                     -o+space  
  -Oz                     -o+space  
  --std=c++??             -cpp  
  --std=gnu++??           -cpp  
  -frtti                  -Ar  
  -fexceptions            -Ae  
  -funsigned-char         -J  
  -fstack-check-generic   -s  
  -fstack-check-specific  -s  
  --ansi                  -A  
  -v                      -v1  
```
