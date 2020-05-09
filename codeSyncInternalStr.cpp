#include "codeSync.hpp"

namespace codeSync{

void printCompileTime()noexcept{
	std::cout<<"\
=====================================================================\n\
   ##  ##    starting   Code Sync   ...\n\
  ##  ##     \n\
 ##  ###### ## ####   Built time: "<<__DATE__<<" "<<__TIME__<<"\n\
  ##    ##  ## #  ##  Repository: https://github.com/mss7z/codeSync\n\
   ##  ##   ## ####\n\
====================================================================="
	<<std::endl;
}

void targetDirFiles::addInternalCsidsIfDef(){
	if(isThereCsidInGlobalTable("__codeSync_selfIntroductionEn")){
		const std::string s{"\
=====================================================================\n\
   ##  ##    This document was written with Code Sync.\n\
  ##  ##     \n\
 ##  ###### ## ####   Generated time: "+timeLib{ch::system_clock::now()}.getStr()+"\n\
  ##    ##  ## #  ##  Repository: https://github.com/mss7z/codeSync\n\
   ##  ##   ## ####\n\
====================================================================="
		};
		addInternalCsid(cdInternalCsid,s);
	}
	if(isThereCsidInGlobalTable("__codeSync_selfIntroductionJp")){
		const std::string s{"\
=====================================================================\n\
   ##  ##    この文書は、Code Syncによって同期されました.\n\
  ##  ##     \n\
 ##  ###### ## ####   生成時間: "+timeLib{ch::system_clock::now()}.getStr()+"\n\
  ##    ##  ## #  ##  リポジトリ: https://github.com/mss7z/codeSync\n\
   ##  ##   ## ####\n\
====================================================================="
		};
		addInternalCsid(cdInternalCsid,s);
	}
	if(isThereCsidInGlobalTable("__codeSync_howToUse")){
		const std::string s{"\
#csid nonestart\n\
## readme.mdからコピーしてきた使い方！\n\
### 基本的な使い方\n\
以下のようなファイルAとBがあったとします。\n\
~~~\n\
#fileA\n\
<--! #csid start hoge -->\n\
私はよく眠りますが、夜は眠れません\n\
<--! #csid end -->\n\
<--! #csid start hoge -->\n\
私はよく眠ります\n\
<--! #csid end -->\n\
~~~\n\
\n\
~~~\n\
#fileB\n\
<--! #csid start hoge -->\n\
私はよく眠ります\n\
<--! #csid end -->\n\
~~~\n\
\n\
このファイルが入ったディレクトリをcodeSyncの引数に指定します。（ディレクトリをcodeSyncにドラッグします。）\n\
すると、次のような状態になります。\n\
~~~\n\
#fileA\n\
<--! #csid start hoge -->\n\
私はよく眠りますが、夜は眠れません\n\
<--! #csid end -->\n\
<--! #csid start hoge -->\n\
私はよく眠りますが、夜は眠れません\n\
<--! #csid end -->\n\
~~~\n\
\n\
~~~\n\
#fileB\n\
<--! #csid start hoge -->\n\
私はよく眠りますが、夜は眠れません\n\
<--! #csid end -->\n\
~~~\n\
\n\
これが、基本的な使い方です。\n\
#csidで始まる部分をコマンドとして理解します。  \n\
#csid start hoge から #csid end まで囲まれた部分を hoge の中身として扱います。\n\
\n\
### 応用的な使い方\n\
以下のようなファイルAとBがあったとします。\n\
~~~\n\
#fileA_oyo\n\
#csid start hoge\n\
  #csid start inner\n\
    入れ子構造にできます\n\
  #csid end\n\
  #csid file unmaster infile\n\
    名前空間に近い考え方です\n\
    file修飾をすると、そのファイル内だけで有効になります\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    unmaster指定すると必ず上書きされてしまいます\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    name指定は入れ子構造の親の中(入れ子でないときはファイル内)だけで有効になります\n\
  #csid end\n\
  #csid name start \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
#csid end\n\
\n\
#csid noneStart\n\
      ^~~~~~~~~\n\
コマンドに大文字小文字は関係ありません(「#csid」は小文字のみ)\n\
#csid start hoge\n\
  私はよく眠ります\n\
#csid end\n\
nonestartとnoneendで囲むとその範囲は処理されません\n\
\n\
#csid_noneEnd\n\
\n\
#csid file master infile\n\
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります\n\
#csid end\n\
\n\
#csid name unmaster \"space s\" name指定をしていますが、名前空間が違うので書き込みされません\n\
#csid end\n\
\n\
#csid masterline line_dayo これはlineです、lineだけはインデントが残ります\n\
	#csid line line_dayo \n\
~~~\n\
\n\
~~~\n\
#fileB_oyo\n\
#csid start hoge 内容がないやつは必ず上書きされます\n\
#csid end\n\
\n\
#csid line line_dayo\n\
~~~\n\
\n\
codeSyncに処理させるとこうなります\n\
\n\
~~~\n\
#fileA_oyo\n\
#csid start hoge\n\
  #csid start inner\n\
    入れ子構造にできます\n\
  #csid end\n\
  #csid file unmaster infile\n\
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
  #csid name start \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
#csid end\n\
\n\
#csid noneStart\n\
      ^~~~~~~~~\n\
コマンドに大文字小文字は関係ありません(「#csid」は小文字のみ)\n\
#csid start hoge\n\
  私はよく眠ります\n\
#csid end\n\
nonestartとnoneendで囲むとその範囲は処理されません\n\
\n\
#csid_noneEnd\n\
\n\
#csid file master infile\n\
  単にstartしたときは、書き換え先を推論しますが、master指定をすると必ずこれがmasterになります\n\
#csid end\n\
\n\
#csid name unmaster \"space s\" name指定をしていますが、名前空間が違うので書き込みされません\n\
#csid end\n\
\n\
#csid masterline line_dayo これはlineです、lineだけはインデントが残ります\n\
	#csid masterline line_dayo これはlineです、lineだけはインデントが残ります\n\
~~~\n\
\n\
~~~\n\
#fileB_oyo\n\
#csid start hoge 内容がないやつは必ず上書きされます\n\
  #csid start inner\n\
    入れ子構造にできます\n\
  #csid end\n\
  #csid file unmaster infile\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
  #csid name unmaster \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
  #csid name start \"space s\"\n\
    空白文字付きのcsidにしたいときは\"\"で囲んでください\n\
  #csid end\n\
#csid end\n\
\n\
#csid masterline line_dayo これはlineです、lineだけはインデントが残ります\n\
~~~\n\
\n\
まとめると、次のような機能があります。  \n\
* 基本はstart~endで囲む\n\
* 自動でバックアップ\n\
	* 同じディレクトリ改装に\"ファイル名_CSBackups\"というディレクトリを作る\n\
	* その中に\"ファイル名_時間\"のディレクトリが作られる\n\
	* その中に操作対象のファイルがバックアップされる\n\
* start~endで囲まれた内容の中でどれを\"master\"にするかを決め、それで上書きして、同期する\n\
	* ユニーク（その名前空間で1つだけ）なstart~endで囲まれた内容を \"master\" として扱う\n\
	* ただし、明示的に\"master\" \"unmaster\"の指定をしたときはそれも考慮する\n\
	* どれを\"master\"にすべきか推論できないときはエラーメッセージが出る\n\
	* \"unmaster\"は必ず、上書きされる（unmasterしかないときは、内容削除）\n\
* line\n\
	* その行だけを同期する\n\
	* \"line\"と\"masterline\"がある（\"unmasterline\"は、その行が消える場合があるのでわざと実装してない）\n\
	* インデントは同期されず、元のが残る\n\
* 入れ子\n\
	* ただし、深さは10000まで（再帰を使ってるのでスタックオーバーフローを防ぐため）\n\
* 名前空間的な何か\n\
	* global ディレクトリ全体で有効(省略時は自動でこれ)\n\
	* name その直近のglobalの親、もしくはfileに属し、その名前空間でのみ有効\n\
	* file そのファイルの中だけで有効(上の例では、fileB_oyoには\"infile\"がなかったので空白になっている)\n\
\n\
このほかにも、私自身が忘れてるやつあるかもしれない  \n\
#csid noneend"
		};
		addInternalCsid(cdInternalCsid,s);
	}
}

}//namespace codeSync