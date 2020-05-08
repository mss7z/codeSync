#include "codeSync.hpp"

namespace{
	void waitForEnter()noexcept;
	void codeShareMain(const std::filesystem::path &taga);
}
int main (int argc,char *argv[]) {
	namespace fs=std::filesystem;
	osSetting();
	checkDebugMode();
	//std::cout<<"hello world!"<<std::endl;
	DBGOUT("hello monkey\n");
	//instantCFLtest("D:\\00_C_Cpp_Program\\codeShare\\tests\\cflDreams.txtccc");
	//return 0;
	try{
		fs::path tagPath;
		if(argc==2){
			tagPath=fs::path(argv[1]);
			tagPath=fs::absolute(tagPath);
			DBGOUT("引数のパス"<<tagPath<<std::endl);
		}else{
			throw std::runtime_error("引数が不正");
		}
		//codeShareMain("D:\\00_C_Cpp_Program\\codeShare\\tests\\");
		codeShareMain(tagPath);
		std::cout<<"hello world!"<<std::endl;
		return 0;
	}catch(const std::exception &e){
		const std::type_info &eType=typeid(e);
		if(eType==typeid(fs::filesystem_error)){
			std::cerr<<"ファイルシステム例外";
		}else if(eType==typeid(std::runtime_error)){
			std::cerr<<"実行時例外";
		}else if(eType==typeid(std::logic_error)){
			std::cerr<<"(たぶん、開発者のせいで)論理例外";
		}else{
			//デマングルしろ
			std::cerr<<"その他の標準例外 \""<<eType.name()<<"\" ";
		}
		std::cerr<<"が発生しました\n説明:"<<e.what()<<std::endl;
	}catch(...){
		std::cerr<<"実装忘れによる予期しない標準以外の例外です\nこれは圧倒的に開発者(おまえ)が悪いです\n彼(自分)に聞いてください"<<std::endl;
	}
	std::cout<<"問題が発生したので、終了するにはEnterキーを押してください"<<std::endl;
	waitForEnter();
	std::exit(EXIT_FAILURE);
}

namespace{
void waitForEnter()noexcept{
	int excCont=0;
	while(true){
		try{
			std::string s;
			std::getline(std::cin,s);
			break;
		}catch(const std::exception &e){
			if(excCont>10){
				std::cerr<<"Enterキーの入力待ちで予期しない例外が発生しました\n説明:"<<e.what()<<"\nプログラムを終了します"<<std::endl;
				std::exit(EXIT_FAILURE);
			}
			std::cin.clear();
		}
		excCont++;
	}
	return;
}
void codeShareMain(const std::filesystem::path &taga){
	using namespace codeSync;
	targetDirFiles dir(taga,{{".txt"}});
	//targetDirFiles dir("tests",{{".txt"},{".exe"}});
	/*dir.tagDir="tests\\";
	//dir.tagDir=".";
	dir.tagExtensions={{".txt"},{".exe"}};*/
	dir.read();
	dir.backup();
	/*for(const auto& i : dir.files){
		std::cout<<i.fsEntry.path()<<std::endl;
		
		//instantCFLtest(i.fsEntry.path());
	}*/
	
	//instantCsidTableViewer(dir.table);
	DBGDO(dir.printTable());
	
	//ここら辺の処理、洗練されてないから未来の自分は何とかしろ
	std::cout<<em<<std::endl;
	if(em.isErr()){
		std::cout<<"エラーがあるため終了します\n終了するにはEnterキーを押すか、×してください"<<std::endl;
		waitForEnter();
		return;
	}
	std::cout<<"処理を続行するにはEnterキーを押してください\nCtrl+Cするか、×すると終了します"<<std::endl;
	
	waitForEnter();
	em.clear();
	dir.write();
	//ここでエラーした時の処理
	std::cout<<em<<std::endl;
	if(em.isErr()){
		throw std::logic_error("errorManagerのerrに予期しない書き込みがありました");
	}else{
		std::cout<<"正常に処理を終了しました\n終了するにはEnterキーを押すか、Ctrl+Cするか、×すると終了します"<<std::endl;
	}
	waitForEnter();
	return;
}
}//namespace