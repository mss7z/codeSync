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
<--! #csid start hoge -->
私はよく眠りますが、夜は眠れません
<--! #csid end -->
<--! #csid start hoge -->
私はよく眠ります
<--! #csid end -->
~~~

~~~
#fileB
<--! #csid start hoge -->
私はよく眠ります
<--! #csid end -->
~~~

このファイルが入ったディレクトリをcodeSyncの引数に指定します。（ディレクトリをcodeSyncにドラッグします。）
すると、次のような状態になります。
~~~
#fileA
<--! #csid start hoge -->
私はよく眠りますが、夜は眠れません
<--! #csid end -->
<--! #csid start hoge -->
私はよく眠りますが、夜は眠れません
<--! #csid end -->
~~~

~~~
#fileB
<--! #csid start hoge -->
私はよく眠りますが、夜は眠れません
<--! #csid end -->
~~~

これが、基本的な使い方です。
#csid で始まる部分をコマンドとして理解します。  
#csid start hoge から #csid end まで囲まれた部分を hoge の中身として扱います。

### 応用的な使い方
以下のようなファイルAとBがあったとします。
~~~
#fileA_oyo
#csid start hoge
  #csid start inner
    入れ子構造にできます
  #csid end
  #csid file unmaster infile
    名前空間に近い考え方です
    file修飾をすると、そのファイル内だけで有効になります
  #csid end
  #csid name unmaster "space s"
    unmaster指定すると必ず上書きされてしまいます
  #csid end
  #csid name unmaster "space s"
    name指定は入れ子構造の親の中(入れ子でないときはファイル内)だけで有効になります
  #csid end
  #csid name start "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
#csid end

#csid noneStart
      ^~~~~~~~~
コマンドに大文字小文字は関係ありません(「#csid」は小文字のみ)
#csid start hoge
  私はよく眠ります
#csid end
nonestartとnoneendで囲むとその範囲は処理されません

#csid noneEnd

#csid file master infile
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります
#csid end

#csid name unmaster "space s" name指定をしていますが、名前空間が違うので書き込みされません
#csid end

#csid masterline line_dayo 
	#csid line line_dayo これはlineです、lineだけはインデントが残ります
~~~

~~~
#fileB_oyo
#csid start hoge 内容がないやつは必ず上書きされます
#csid end

#csid line line_dayo
~~~

codeSyncに処理させるとこうなります

~~~
#fileA_oyo
#csid start hoge
  #csid start inner
    入れ子構造にできます
  #csid end
  #csid file unmaster infile
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります
  #csid end
  #csid name unmaster "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
  #csid name unmaster "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
  #csid name start "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
#csid end

#csid noneStart
      ^~~~~~~~~
コマンドに大文字小文字は関係ありません(「#csid」は小文字のみ)
#csid start hoge
  私はよく眠ります
#csid end
nonestartとnoneendで囲むとその範囲は処理されません

#csid noneEnd

#csid file master infile
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります
#csid end

#csid name unmaster "space s" name指定をしていますが、名前空間が違うので書き込みされません
#csid end

#csid masterline line_dayo 
	#csid masterline line_dayo 
~~~

~~~
#fileB_oyo
#csid start hoge 内容がないやつは必ず上書きされます
  #csid start inner
    入れ子構造にできます
  #csid end
  #csid file unmaster infile
  #csid end
  #csid name unmaster "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
  #csid name unmaster "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
  #csid name start "space s"
    空白文字付きのcsidにしたいときは""で囲んでください
  #csid end
#csid end

#csid masterline line_dayo 
~~~
