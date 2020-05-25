# codeSync
HTMLファイルの共通部分を同期するために作りました。  
引数に指定したディレクトリの ".txt",".html",".htm",".css" の拡張子を読み取ります。  
自分で使うことしか考えていないので、雑魚です。
UTF-8以外だと、マルチバイト文字で表示が文字化けします。（生成されるファイルには影響ないはずです）
## 経緯
プログラミングを学習している学生です。  
モダンなC++を学習する目的もあって、作ったプログラムです。  
以前まで書いていたのはC++98かC++03相当のものだとわかり、まずいなと思ったので休校期間を利用して、C++17で書きました。  
モダンなC++は、使い始めて1カ月くらいしかたってないので、こういう書き方はよくないなどあったら、ぜひ教えてください。  
Makefileも初めて書いたので、よくない使い方などしていたら、教えてください。  
Gitも、まともに使ったことがなくて、謎です。
## コンパイル方法

~~~
make
~~~

もしくは

~~~
g++ -std=c++17 -Wall -Wextra -pedantic-errors -DNDEBUG   -c -o codeSyncMain.o codeSyncMain.cpp
g++ -std=c++17 -Wall -Wextra -pedantic-errors -DNDEBUG   -c -o codeSyncClasses.o codeSyncClasses.cpp
g++ -std=c++17 -Wall -Wextra -pedantic-errors -DNDEBUG   -c -o codeSyncInternalStr.o codeSyncInternalStr.cpp
g++ codeSyncMain.o codeSyncClasses.o codeSyncInternalStr.o -static -lstdc++ -lgcc -o codeSync.exe
~~~

次のversionでのコンパイルを確認しています
> g++ (Rev1, Built by MSYS2 project) 9.3.0

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

#csid masterline line_dayo これはlineです、lineだけはインデントが残ります
	#csid line line_dayo 
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

#csid masterline line_dayo これはlineです、lineだけはインデントが残ります
	#csid masterline line_dayo これはlineです、lineだけはインデントが残ります
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

#csid masterline line_dayo これはlineです、lineだけはインデントが残ります
~~~

まとめると、次のような機能があります。  
* 基本はstart~endで囲む
* 自動でバックアップ
	* 同じディレクトリ改装に"ファイル名_CSBackups"というディレクトリを作る
	* その中に"ファイル名_時間"のディレクトリが作られる
	* その中に操作対象のファイルがバックアップされる
* start~endで囲まれた内容の中でどれを"master"にするかを決め、それで上書きして、同期する
	* ユニーク（その名前空間で1つだけ）なstart~endで囲まれた内容を "master" として扱う
	* ただし、明示的に"master" "unmaster"の指定をしたときはそれも考慮する
	* どれを"master"にすべきか推論できないときはエラーメッセージが出る
	* "unmaster"は必ず、上書きされる（unmasterしかないときは、内容削除）
* line
	* その行だけを同期する
	* "line"と"masterline"がある（"unmasterline"は、その行が消える場合があるのでわざと実装してない）
	* インデントは同期されず、元のが残る
* 入れ子
	* ただし、深さは10000まで（再帰を使ってるのでスタックオーバーフローを防ぐため）
* 名前空間的な何か
	* global ディレクトリ全体で有効(省略時は自動でこれ)
	* name その直近のglobalの親、もしくはfileに属し、その名前空間でのみ有効
	* file そのファイルの中だけで有効(上の例では、fileB_oyoには"infile"がなかったので空白になっている)
* 処理させない機能
	* #csid none と行の最初に書くとその行にほかに#csid があっても処理されない
	* #csid nonestart ~ #csid noneend で囲まれた範囲は処理されない
	* ちなみに#csid の後ろに空白文字がない時も処理されない  
このほかにも、私自身が忘れてるやつあるかもしれない  
