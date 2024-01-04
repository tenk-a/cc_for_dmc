cmake を用いたサンプル

dmc を使えるようにパス設定しておく。

windows 用の gnu make を make.exe のファイル名で、dmc フォルダより先に  
検索されるパスに置いておく。

余計な dll 不要で使える MinGW32-make.exe や GnuWin32 Make が無難。
※ dmc が link.exe を使う都合、競合が怖いので、mingw, msys, cygwin 等の
   環境にパスを通すのは避ける。

GnuWin32 make なら
  https://gnuwin32.sourceforge.net/packages/make.htm
のページのバイナリの zip をダンロードして展開。

お試しで無精するなら、cc.exe のある ../bin フォルダに make.exe を仮置きするのでも。

cc4dmc_cmake.bat

を実行。
