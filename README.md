# cc for dmc

gcc/cc風のコマンドライン引数で dmc でコンパイルするためのラッパー。  
オプション文字列の変換とファイルパスの '/' を '\\' に変換する。  
また特に指定がなければリンカとして link.exe でなく optlink.exe を使う。

cmake ＆ gnu make を使って dmc でのコンパイルを試しみるために用意。

## 利用方法

- bin\dmc-cc.exe を dmc.exe のあるフォルダに入れる
- 環境変数 DMC_DIR か DMC に dmc ディレクトリを設定しておく
- dmc ディレクトリを c:\dm、c:\DMC\dm、c:\dmc にする  
　　　　※ \dm　　　CD版、DL版をそのままc:に置いた場合.  
　　　　　 \DMC\dm　dmd の installer で一緒に install した場合.  
　　　　　 \dmc　　 https://github.com/digitalmars/dmc をままcloneした場合.

のいずれかを行う。

## ビルド

https://github.com/digitalmars/dmc.git

を clone した dmc 環境を使う。  
(CD版相当のようなので、16bit exe 作成やその当時のIDEやデバッガ付属)  
あるいは dmd同時インストールや DigitalMars サイトのDL版
の 32bit 最小構成でもよく。

(Dmc-Dir)\bin\dmc.exe へのパスが通った状態で、bld\mk.bat を実行。

## Usage

```
usage> dmc-cc.exe [-options] filename(s)
      Convert and pass gcc-like command line arguments to dmc.
      Filename convert '/' to '\'.
  @FILE     Input response FILE.
  --help    Help.
  --NATIVE  Afterwards dmc option.
  --GCC     Afterwards gcc option.
 (gcc)                   (dmc)
  --define-macro M[=S]    -D[M[=S]]
  -D[MACRO[=STR]]         -D[MACRO[=STR]]
  --undefine-macro MACRO  -U[MACRO]
  -U[MACRO]               -U[MACRO]
  --include-directory DIR -I[DIR]
  -I DIR                  -I[DIR]
  --include FILE          -HI[FILE]
  --output FILE           -o[FILE]
  -o FILE                 -o[FILE]
  --library NAME          lib[NAME].lib
  -l NAME                 lib[NAME].lib
  --library-path DIR      -L/DIR
  -L DIR                  -L/DIR
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
  --std=c??               
  --std=gnu??             
  -frtti                  -Ar
  -fexceptions            -Ae
  -funsigned-char         -J
  -fsigned-char           
  -fstack-check-generic   -s
  -fstack-check-specific  -s
  --ansi                  -A
  -v                      -v1
```

## cmake & gnu make

cmake で -G "Unix Makefiles" か "MinGW32 Makefiles" で
お試しできるように作成。  

gnu make は、dllの柵なく単体利用できる GnuWin32 や MinGW32-make を想定。  
名前は make.exe にしておく。

一応 dmc のリンカとして link.exe でなく optlink.exe を使う設定にするので、
dmc より優先して msys 等のパスを通せば その make が使えるかも。

使う場合は、予め、環境変数

set CC=dmc-cc.exe  
set CXX=dmc-cc.exe

をしてから cmake すること。

cmake の引数で指定した -DCMAKE_C_COMPILER や -DCMAKE_CXX_COMPILER が有効になる前に、
別途 -G "MinGW32 Makefiles" 側のコンパイラの動作テストが行われるようで、
環境変数で指定しないとコンパイラを置き換えられなかった。  
※環境変数なしだと cl.exe が gccのオプション形式で使われるという奇妙な状態になる…

簡単なオプションの変換しかしておらず、ライブラリ関係や拡張子等いろいろ違いがあるので、
gccままの設定でビルドを通せるわけでないので、夢はみないように、と。
