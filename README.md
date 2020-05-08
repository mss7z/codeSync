# codeSync
HTMLファイルの共通部分を同期するために作りました。
## 経緯
モダンなC++を勉強しながら、作ったプログラムです。
以前まで書いていたのはC++98相当のものだとわかり、まずいなと思ったので休校期間を利用して、C++17で書きました。
こういう、書き方はよくないなどあったら、ぜひ教えてください。
## 使用法
### 基本的な使い方
以下のようなファイルAとBがあったとします。
~~~
#fileA
#csid start hoge
私はよく眠りますが、夜は眠れません
#csid end
#csid start hoge
私はよく眠ります
#csid end
~~~

~~~
#fileB
#csid start hoge
私はよく眠ります
#csid end
~~~

このファイルが入ったディレクトリをcodeSyncの引数に指定します。（ディレクトリをcodeSyncにドラッグします。）
すると、次のような状態になります。
~~~
#fileA
#csid start hoge
私はよく眠りますが、夜は眠れません
#csid end
#csid start hoge
私はよく眠りますが、夜は眠れません
#csid end
~~~

~~~
#fileB
#csid start hoge
私はよく眠りますが、夜は眠れません
#csid end
~~~

これが、基本的な使い方です。
