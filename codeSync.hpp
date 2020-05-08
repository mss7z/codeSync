#ifndef CODE_SYNC_HPP_INCLUDE_GUARD
#define CODE_SYNC_HPP_INCLUDE_GUARD

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <deque>
#include <list>
#include <string>
#include <filesystem>
#include <algorithm>
#include <memory>
#include <ctime>
#include <cstdlib>
#include <cassert>


#if _WIN64 || _WIN32
	inline void osSetting(){std::system("chcp 65001 > nul");}
#else
	inline void osSetting(){std::cout<<"hello linux?"<<std::endl;}
#endif

#ifdef NDEBUG
	inline void checkDebugMode(){}
	#define DBGOUT(X) do{}while(false)
	#define DBGOUTLN(X) do{}while(false)
	#define DBGDO(X) do{}while(false)
	#define SVAL(X)
#else
	inline void checkDebugMode(){
		std::cout<<"注意: デバック版\n"
		<<"コンパイル日時: "<<__DATE__<<" "<<__TIME__<<std::endl;
	}
	#define DBGOUT(X) do{std::cout<<X;}while(false)
	#define DBGOUTLN(X) do{std::cout<<X<<std::endl;}while(false)
	#define DBGDO(X) do{X;}while(false)
	#define SVAL(X) #X<<':'<<(X)<<' '
#endif

namespace codeSync{

namespace fs = std::filesystem;
namespace ch = std::chrono;
inline std::string& str2lower(std::string &s){
	std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){
		return std::tolower(c);
	});
	return s;
}


class noCopyable{
	protected:
	noCopyable()=default;
	~noCopyable()=default;
	noCopyable(const noCopyable&)=delete;
	noCopyable& operator=(const noCopyable&)=delete;
};
class noMovable{
	protected:
	noMovable()=default;
	~noMovable()=default;
	noMovable(noMovable&&)=delete;
	noMovable& operator=(noMovable&&)=delete;
};
class noMovCopyable:
	protected noCopyable,
	protected noMovable
{};

class [[nodiscard]] fileError{
	private:
	const std::string s;
	public:
	fileError(const std::string &sa):s(sa){};
	fileError(const char sa[]):s(sa){};
	friend std::ostream& operator<<(std::ostream &st,const fileError &e){st<<e.s;return st;}
	operator std::string()const{return s;}
	const std::string& getStr()const{return s;}
};

//////////////////////// errorManager
class errorManager:noMovCopyable{
	//エラーやインフォメーションをstreamに記憶しておく
	private:
	class css:noMovCopyable,
		public std::stringstream
	{
		private:
		errorManager *parentp;
		public:
		css(errorManager *parenta);
		void resetSS();
		void writeTo(std::ostream&,const std::string&)const;
		~css();
	};
	std::vector<css*> cssLocker;
	public:
	constexpr static const char *cont=":\n\t";
	constexpr static const char *add="\n\t";
	constexpr static const char *indent="\n\t\t";
	constexpr static const char *dot="* ";
	css err{this},warn{this},info{this},chk{this};
	void clear();
	bool isErr()const{return !(err.str().empty());}
	friend std::ostream& operator<<(std::ostream&,const errorManager&);
};
extern errorManager em;
inline void errorManager::css::writeTo(std::ostream &stream,const std::string &s)const{
	if(!str().empty()){
		stream<<s<<'\n'<<rdbuf()<<'\n';
	}
}
inline void errorManager::clear(){
	for(auto &i:cssLocker){
		i->resetSS();
	}
}

//////////////////////// timeLib
template<typename TP>
class timeLib {
	//C++20になるといらなくなる？クラス
	//異なったエポック、ratioを持つclockをsystem_clock::time_pointに変換して保持
	private:
	const ch::system_clock::time_point tp;
	const std::time_t tt;
	static std::tm localtimeGen(const std::time_t &tta){
		std::tm temp;
		localtime_s(&temp,&tta);
		return temp;
	}
	const std::tm tmt;
	
	public:
	timeLib(const TP tpa);
	time_t getTime_t()const {return tt;}
	std::string getStr()const;
	std::string getStrOnlyNum()const;
};
using ftimeLib = timeLib<fs::file_time_type>;

//////////////////////// infoType
class infoTypeBase{
	public:
	virtual std::string getStr()const=0;
	template<class IT>//info type
	static std::string getStrFrom(const std::vector<IT>&);
	virtual ~infoTypeBase()=default;
};
class [[nodiscard]] infoType:public infoTypeBase{
	private:
	std::shared_ptr<const infoTypeBase> itp;
	public:
	infoType(const infoTypeBase*);
	explicit infoType(const infoTypeBase&);
	template<class IT,typename... ARGS>
	friend infoType infoTypeGen(ARGS... args);
	template<class IT>
	friend infoType infoTypeCast(IT&&);
	std::string getStr()const override;
};

template<class IT,typename... ARGS>
inline infoType infoTypeGen(ARGS... args){
	return infoType{new IT {args...}};
}
template<class IT>
inline infoType infoTypeCast(IT &&r){
	return infoType{new typename std::remove_const<typename std::remove_reference<decltype(r)>::type>::type{std::forward<IT>(r)}};
}

inline std::string infoType::getStr()const{
	return itp->getStr();
}

using infoTypeCastable=infoTypeBase;
class [[nodiscard]] fileInfo:public infoTypeCastable{
	//その名の通りファイルの情報
	private:
	struct fileInfoBody{
		std::string fileName;
		ftimeLib fileTime;
		fileInfoBody(const std::string &fileNamea,const ftimeLib &fileTimea):
			fileName(fileNamea),fileTime(fileTimea){}
	};
	static std::vector<fileInfoBody> bodys;
	static decltype(bodys)::size_type getIndexAndAddBodys(const std::string &namea,const ftimeLib &timea);
	decltype(bodys)::size_type index;
	const std::string& refFileName()const;
	const ftimeLib& refFileTime()const;
	public:
	auto getIndex()const{return index;}
	fileInfo(const std::string &namea,const ftimeLib &timea):
		index(getIndexAndAddBodys(namea,timea)){}
	fileInfo(const fileInfo &f):
		index(f.getIndex()){}
	std::string getStrNoAt()const;
	std::string getStr()const override;
	
	
};

inline std::vector<fileInfo::fileInfoBody>::size_type fileInfo::getIndexAndAddBodys(const std::string &namea,const ftimeLib &timea){
	const std::vector<fileInfoBody>::size_type index=bodys.size();
	bodys.emplace_back(namea,timea);
	return index;
}
inline const std::string& fileInfo::refFileName()const{
	return bodys[index].fileName;
}
inline const ftimeLib& fileInfo::refFileTime()const{
	return bodys[index].fileTime;
}
inline std::string fileInfo::getStrNoAt()const{
	return "\""+refFileName()+"\" ("+refFileTime().getStr()+")";
}
inline std::string fileInfo::getStr()const{
	return "@ "+getStrNoAt();
}

struct [[nodiscard]] lineInfo:public infoTypeCastable{
	//その名の通り行番号とその内容の情報
	int lineNum;
	std::string lineStr;
	lineInfo(const int numa,const std::string &stra):
		lineNum(numa),lineStr(stra){}
	lineInfo(const lineInfo &l):
		lineInfo(l.lineNum,l.lineStr){}
	lineInfo& operator=(const lineInfo &l){
		lineNum=l.lineNum;
		lineStr=l.lineStr;
		return *this;
	}
	lineInfo():lineNum(0){}
	
	std::string getStr()const override;
};

inline std::string lineInfo::getStr()const{
	std::stringstream ss;
	//8は、行数表示の桁そろえ
	ss<<std::setw(8)<<lineNum<<" | "<<lineStr;
	return ss.str();
}

struct lineType:public lineInfo{
	private:
	bool isAvailableVal=true;
	public:
	bool isAvailable()const{return isAvailableVal;}
	void unavailable(){isAvailableVal=false;}
};
class [[nodiscard]] doubleInfo:public infoTypeCastable{
	//二つの情報を入れて改行区切りで表示
	private:
	const infoType mother;
	const infoType child;
	public:
	doubleInfo(const infoType &filea,const infoType &childa):
		mother(filea),child(childa){}
	std::string getStr()const override;
};
inline std::string doubleInfo::getStr()const{
	return mother.getStr()+"\n"+child.getStr();
}

class [[nodiscard]] stringInfo:public infoTypeCastable{
	private:
	const std::string s;
	public:
	stringInfo(const std::string&);
	std::string getStr()const override;
};
inline stringInfo::stringInfo(const std::string &sa):
	s(sa){}
inline std::string stringInfo::getStr()const{DBGOUTLN("milebb");
	return s;
}

//////////////////////// streamLocker
class streamLockerClass{
	//classのメンバにstream乗っけてたら面白いことになったから
	private:
	std::vector<std::fstream*> fstreams;
	auto close(std::fstream *tag);
	public:
	std::fstream* getP(fs::path path);
	~streamLockerClass();
};
extern streamLockerClass streamLocker;

//////////////////////// LINE!!!
//ここにあるクラスは、streamなどから、行を一行ずつ、書き込み、読み込みをする
class lineBaseClass{
	protected:
	lineBaseClass()=default;
	public:
	virtual ~lineBaseClass()=default;
};
class lineReader:virtual public lineBaseClass{
	public:
	virtual void resetBeforeRead()=0;
	virtual const lineType& getNext()=0;
	virtual const lineType& getNow()=0;
};
class lineWriter:virtual public lineBaseClass{
	public:
	virtual void resetBeforeWrite()=0;
	virtual void addNext(const std::string&)=0;
};
class lineRW:
	public lineReader,
	public lineWriter
{};

class lineStreamReader:
	public lineRW
{
	//streamを行単位で読み書き
	private:
	std::iostream *stp;
	lineType li;
	bool isStreamEOS()const;
	bool isLastLine;//getNext以外での使用は保証しない
	bool isFirstLine;//addNext以外での使用は保証しない
	
	public:
	std::streambuf* getRdbuf()const{return stp->rdbuf();}
	int getLoadedLineNum()const{return li.lineNum;}
	lineStreamReader(std::iostream*);
	void gotoLine(int);
	const lineType& getNext()override;
	const lineType& getNow();
	void addNext(const std::string&)override;
	void resetSt();
	void resetBeforeRead()override{resetSt();}
	//void resetBeforeWrite()override{resetBeforeRW();}
};

inline bool lineStreamReader::isStreamEOS()const{
	return stp->eof()||stp->fail();
}
class lineFileReader:public lineStreamReader{
	//fileを行単位で読み書き
	private:
	std::fstream *f;
	const fs::path path;
	void chkOpened();
	std::string getSelfName() const {return "File \""+path.string()+"\"";}
	public:
	lineFileReader(fs::path);
	void reopenAs(std::ios_base::openmode mode=std::ios_base::in | std::ios_base::out);
	void delContent();
	void resetBeforeWrite()override;
};
inline void lineFileReader::delContent(){
	reopenAs(std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
}

class lineSeekReader:public lineReader{
	//任意のlineStreamReaderに寄生し、その行の区間を使う(std::string_viewに近い思考)
	//だけど、現在使っていません
	private:
	const lineType emptyLi;
	int startLineNum=0,endLineNum=0,markLineNum=0;
	lineStreamReader &lordLL;
	public:
	lineStreamReader& getLordLL()const{return lordLL;}
	lineSeekReader(lineStreamReader&);
	lineSeekReader(lineSeekReader&);
	const lineType& getNext()override;
	void resetBeforeRead()override;
	
	void captureStart(int offset=1);
	void captureEnd(int offset=-1);
	void captureMark(int offset=1);
	void restoreMark();
	
	//friend lineSeekReader;
};
class lineStringReader:public lineStreamReader{
	//stringstreamを行単位で読み書き
	private:
	std::stringstream ss;
	public:
	lineStringReader(lineStreamReader&);
	lineStringReader(const std::string&);
	void resetBeforeWrite()override;
	void print(){std::cout<<"csidStringReader-----\n"<<ss.str()<<std::endl;}
};

//////////////////////// csidType
class csidDictHash{
	//csidTypeの中で検索を早くするために使う
	private:
	//13の意味についてはcsidDictHash::hashFuncを参照
	constexpr static int hashTableSize=0b1<<13;
	constexpr static int hashTableMask=hashTableSize-1;
	int hashTable[hashTableSize];
	const int *hashTableEndP;
	bool isFullTable=false;
	int hashFunc(const std::string&);
	int *tableP;
	public:
	constexpr static int NONE=-1,FULL=-2;
	csidDictHash();
	int getCandidate(const std::string&);
	int nextCandidate();
	void setCandidate(const int);
};
inline int csidDictHash::hashFunc(const std::string &s){
	//hashTableMaskでマスクして得られる領域(計13bit)
	//0b 0000              000 0000                00
	//   ^~~~              ^~~~~~~~                ^~
	//   csidのサイズ由来  csidの最初の文字由来    予備領域
	assert((((s.size()&0xF)<<9)|((static_cast<int>(*s.c_str())&0x7F)<<2))<hashTableSize);
	return (((s.size()&0xF)<<9)|((static_cast<int>(*s.c_str())&0x7F)<<2))&hashTableMask;
}
inline int csidDictHash::getCandidate(const std::string &s){
	tableP=hashTable+hashFunc(s);
	return *tableP;
}
inline int csidDictHash::nextCandidate(){
	tableP++;
	if(!(tableP<hashTableEndP)){
		if(isFullTable){
			return FULL;
		}
		isFullTable=true;
		tableP=hashTable;
	}
	return *tableP;
}
inline void csidDictHash::setCandidate(const int v){
	*tableP=v;
}
	
class csidType{
	//csidを記録しておく
	private:
	static std::vector<std::string> dict;//途中から値を加えてはならない
	public:
	using indexType=decltype(dict)::size_type;
	private:
	static csidDictHash hash;
	static indexType findSameElseAdd2Dict(const std::string&);
	static const indexType emptyIndex;
	indexType index;
	public:
	static const csidType emptyCsid;
	static indexType getCsidTotal(){return dict.size();}
	indexType getIndex()const{return index;}
	csidType();
	csidType(const std::string&);
	csidType(const csidType&);
	std::string& getStr()const;
	std::string getStrCsid()const;
	std::string getStrQuote()const;
	void clear();
	bool empty();
	//explicit
	csidType& operator=(const std::string&);
	csidType& operator=(const csidType&);
	friend std::ostream& operator<<(std::ostream&,const csidType&);
	friend std::istream& operator>>(std::istream&,csidType&);
	operator std::string()const;
	operator indexType()const;
	operator int()const=delete;
	bool operator==(const csidType&)const;
	friend std::string operator+(const std::string&,const csidType&);
};
inline std::string& csidType::getStr()const{
	return dict[index];
}
inline std::string csidType::getStrCsid()const{
	return std::move("CSID \""+dict[index]+'\"');
}
inline std::string csidType::getStrQuote()const{
	return std::move('\"'+dict[index]+'\"');
}
inline void csidType::clear(){
	index=emptyIndex;
}
inline bool csidType::empty(){
	return index==emptyIndex;
}
inline csidType& csidType::operator=(const std::string &s){
	index=findSameElseAdd2Dict(s);
	return *this;
}
inline csidType& csidType::operator=(const csidType &v){
	index=v.getIndex();
	return *this;
}
inline bool csidType::operator==(const csidType &v)const{
	return index==v.getIndex();
}
inline std::string operator+(const std::string &s,const csidType &v){
	return std::move(s+v.getStr());
}
template<typename T>
class csidFinderBase{
	protected:
	std::vector<T> table;
	const T none;
	using tableST=typename decltype(table)::size_type;
	public:
	csidFinderBase(const tableST,const T&);
	
	virtual void set(const csidType&,const T&)=0;
	virtual typename std::conditional<std::is_same<T,bool>::value,bool,const T&>::type 
		find(const csidType&)const=0;
	void unset(const csidType&);
	void clear(){table.clear();}
};
template<typename T>
class flexibleCsidFinder:public csidFinderBase<T>{
	private:
	using tableST=typename csidFinderBase<T>::tableST;
	constexpr static const int tableAddSizeWhenResize=50;
	public:
	flexibleCsidFinder(const T&);
	void set(const csidType&,const T&)override;
	typename std::conditional<std::is_same<T,bool>::value,bool,const T&>::type 
		find(const csidType&)const override;
};
template<typename T>
class constCsidFinder:public csidFinderBase<T>{
	private:
	using tableST=typename csidFinderBase<T>::tableST;
	public:
	constCsidFinder(const T&);
	void set(const csidType&,const T&)override;
	typename std::conditional<std::is_same<T,bool>::value,bool,const T&>::type 
		find(const csidType&)const override;
};
template<typename T>
inline void constCsidFinder<T>::set(const csidType &toa,const T &vala){
	this->table.at(static_cast<tableST>(toa))=vala;
}
template<typename T>
inline auto constCsidFinder<T>::find(const csidType &toa)const
	->typename std::conditional<std::is_same<T,bool>::value,bool,const T&>::type
{
	return this->table.at(static_cast<tableST>(toa));
}
template<class PC>


class csidIsFinder:public PC{
	public:
	csidIsFinder();
	void set(const csidType&);
};
template<class PC>
inline csidIsFinder<PC>::csidIsFinder():
	PC(false){}
template<class PC>
inline void csidIsFinder<PC>::set(const csidType &toa){
	PC::set(toa,true);
}
using constCsidIsFinder=csidIsFinder<constCsidFinder<bool>>;
using flexibleCsidIsFinder=csidIsFinder<flexibleCsidFinder<bool>>;


//////////////////////// csidReader(RW)
//ここのクラスは、上記のlineReaderを使って、文の構造を読み取る
struct csidKeyword{
	enum status{ORDINARILY,START,LINE,END,NONE_START,NONEC,EGG,NONE_END,EOS,UNKNOWN_STS};
	enum option{NONE,MASTER,UNMASTER,UNKNOWN_OPT};
	enum csidNamespace{GLOBAL,NAME,FILE,UNKNOWN_CNS};
};

struct analyzedLine:public csidKeyword{
	status sts;
	csidType csid;
	option opt;
	csidNamespace cns;
	analyzedLine(const status stsa,const csidType &csida,const option opta,const csidNamespace cnsa):
		sts(stsa),csid(csida),opt(opta),cns(cnsa){}
	analyzedLine():
		sts(UNKNOWN_STS),csid(csidType::emptyCsid),opt(UNKNOWN_OPT),cns(UNKNOWN_CNS){}
};

class csidReaderBase:public csidKeyword{
	public:
	virtual const std::string& getLineStr()=0;
	virtual const csidType& getCsid()=0;
	
	virtual void resetBeforeNextLine()=0;
	virtual status nextLine()=0;
	virtual csidNamespace getCsidNamespace()=0;
	virtual ~csidReaderBase()=default;
};
class csidReader4Read:virtual public csidReaderBase{
	public:
	virtual option getOpt()=0;
	virtual const lineInfo& getLineInfo()=0;
	virtual analyzedLine getAnalyzedLine()=0;
	virtual std::string_view getLineMain()=0;
};
//using csidReader4Write=csidReaderBase;
class csidReader4Write:virtual public csidReaderBase{
	public:
	virtual std::string_view getLineIndent()=0;
};


class csidLineReader:public csidReader4Read{
	//一行ずつ読み取ってcsidを解析する
	private:
	constexpr static const char csidTag[]="#csid ";
	constexpr static const int csidTagSize=sizeof(csidTag)-1;//スペース分を引く
	std::stringstream csidSeparaterSS;
	[[nodiscard]] bool isPracticableNextLine();
	[[nodiscard]] bool isPracticableCsidSeparaterSS();
	[[nodiscard]] status getCmdFromCsidSeparaterSS();
	[[nodiscard]] status getCmdSwitchWhenNone(const std::string &cmd)noexcept;
	[[nodiscard]] status getCmdSwitchWhenEnable(const std::string &cmd)noexcept;
	csidNamespace getCsidNamespaceSwitch(const std::string &cns)noexcept;
	void loadCsidFromCsidSeparaterSS();
	csidType csid;
	lineType line;
	status cmd;
	csidNamespace cns;
	option opt=NONE;
	lineReader *const ll;
	bool isNoneCmdPart=false;
	public:
	
	csidLineReader(lineReader*);
	csidLineReader(lineReader&);
	const lineReader& getLineReader(){return (*ll);}
	const std::string& getLineStr()override{return line.lineStr;}
	const csidType& getCsid()override{return csid;}
	analyzedLine getAnalyzedLine()override;
	
	option getOpt()override{return opt;}
	csidNamespace getCsidNamespace()override{return cns;}
	int getLineNum(){return line.lineNum;}
	void resetBeforeNextLine()override;
	status nextLine()override;
	const lineInfo& getLineInfo()override;
	std::string_view getLineMain()override;
	
	csidLineReader& operator=(const csidLineReader&)=delete;
	csidLineReader& operator=(csidLineReader&&)=delete;
};
inline const lineInfo& csidLineReader::getLineInfo(){
	return line;
}

class csidLineReaderRapChecker:public csidLineReader{
	//csidLineReaderは一行しか見ないが、行をまたいだエラーを検知する
	private:
	constexpr static const int maxInLevel=10'000;
	flexibleCsidIsFinder finder;
	std::deque<csidType> inStack;
	std::string getDependentStr()const;
	public:
	using csidLineReader::csidLineReader;
	void resetBeforeNextLine();
	status nextLine();
};
inline void csidLineReaderRapChecker::resetBeforeNextLine(){
	csidLineReader::resetBeforeNextLine();
	inStack.clear();
	finder.clear();
}

//////////////////////// csidContent
//ここは、csidの内容(content)を記録するためのクラス
class csidContent{
	//csidContentの基本となるクラス
	private:
	std::vector<std::string> strs;
	protected: //strsの途中から要素が追加されてはいけない
	void addToStrs(const std::string &s){strs.push_back(s);}
	int addToStrsWithNum(const std::string &s);
	void eraseStrs(){strs.erase(strs.begin(),strs.end());}
	public:
	const decltype(strs)& refStrs()const{return strs;}
	csidContent(){DBGOUTLN("csidContentが作られた");}
	csidContent(csidContent&&)noexcept;
	csidContent(const csidContent&)noexcept;
	csidContent& operator=(csidContent&&)noexcept;
	bool operator==(const csidContent&)const;
	friend std::ostream& operator<<(std::ostream&,const csidContent&);
	bool isEmpty()const;
	virtual ~csidContent()=default;
};
inline bool csidContent::operator==(const csidContent &a)const{
	return strs==a.strs;
}
inline bool csidContent::isEmpty()const{
	return strs.size()==0;
}

class csidContentLineWriter:
	public csidContent
{
	//csidContentに1行だけの書き込みを行う
	public:
	csidContentLineWriter(const std::string&);
};
class csidContentLineReader{
	//csidContentから1行だけの読み込みを行う
	public:
	const csidContent& lordCC;
	public:
	csidContentLineReader(const csidContent&);
	const std::string& getStr()const;
};
inline const std::string& csidContentLineReader::getStr()const{
	return (lordCC.refStrs())[0];
}

class csidContentPartWriter:
	public csidContent,
	public lineWriter
{
	//csidContentに書き込みを行う
	public:
	void resetBeforeWrite()override;
	void addNext(const std::string&)override;
};
inline void csidContentPartWriter::resetBeforeWrite(){DBGOUTLN("milessss");
	eraseStrs();
}
inline void csidContentPartWriter::addNext(const std::string &str){
	addToStrs(str);
}
class lineCsidContentReader:
	public lineReader
{
	//csidContentから読み込みを行う
	private:
	const csidContent& lordCC;
	std::vector<std::string>::const_iterator itr;
	lineType li;
	public:
	lineCsidContentReader(const csidContent&);
	void resetBeforeRead()override;
	const lineType& getNext()override;
};
inline void lineCsidContentReader::resetBeforeRead(){
	itr=lordCC.refStrs().begin();
	li.lineNum=0;
}
inline const lineType &lineCsidContentReader::getNext(){
	if(itr==lordCC.refStrs().end()){
		li.unavailable();
		return li;
	}
	li.lineStr=(*itr);
	li.lineNum++;
	itr++;
	return li;
}


class csidContentDetail:
	public csidContent,
	public csidKeyword
{
	//基本となるcsidContentに加えて、文構造の解析結果も保存する
	public:
	struct infosType{
		int refNum;
		analyzedLine analyzed;
		infosType(const int refNuma,const analyzedLine &analyzeda):
			refNum(refNuma),analyzed(analyzeda){}
		infosType(const infosType &v):
			infosType(v.refNum,v.analyzed){}
		infosType(infosType &&v)noexcept:
			refNum(std::move(v.refNum)),analyzed(std::move(v.analyzed)){}
	};
	protected:
	std::vector<infosType> infos;
	
	public:
	csidContentDetail()=default;
	csidContentDetail(const csidContentDetail&)noexcept;
	csidContentDetail(csidContentDetail&&)noexcept;
	csidContentDetail& operator=(csidContentDetail&&)=delete;
	csidContentDetail& operator=(const csidContentDetail&)=delete;
	const decltype(infos)& refInfos()const{return infos;}
};
class csidContentDetailWriter:public csidContentDetail{
	//csidContentDetailに書き込みを行う
	public:
	void addLine(const std::string&,const analyzedLine);
	void addLine(const std::string&);
};
inline void csidContentDetailWriter::addLine(const std::string &s){
	addToStrs(s);
}
class csidContentDetailReader:public csidKeyword{
	//csidContentDetailから読み込みを行う
	private:
	using infosType=csidContentDetail::infosType;
	const csidContentDetail &lordCCD;
	std::vector<infosType>::const_iterator infosItr;
	bool isInfosItr;
	int strsNumCont;
	
	static const analyzedLine eosAnalyzed;
	static const analyzedLine ordinaryAnalyzed;
	static const std::string emptyStr;
	
	public:
	csidContentDetailReader(const csidContentDetail&);
	void reset();
	std::pair<const std::string&,const analyzedLine&> getLine();
};
class csidCsidContentDetailReader:
	public csidReader4Write
{
	//名前が紛らわしいが、csidContentDetailReaderを使って、csidReaderのようにふるまう
	//csid csidContentDetail Reader
	private:
	csidContentDetailReader &lordCCDR;
	const analyzedLine *analyzedP;
	const std::string *lineStrP;
	public:
	csidCsidContentDetailReader( csidContentDetailReader&);
	
	const std::string& getLineStr()override;
	const csidType& getCsid()override;
	
	void resetBeforeNextLine()override;
	status nextLine()override;
	csidNamespace getCsidNamespace()override;
	std::string_view getLineIndent()override;
	
	csidCsidContentDetailReader& operator=(const csidCsidContentDetailReader&)=delete;
	csidCsidContentDetailReader& operator=(csidCsidContentDetailReader&&)=delete;
};

//////////////////////// TABLE!!!!!
struct tableCsidsKeyword{
	enum option{NONE,MASTER,UNMASTER,INTERNAL,INT_OPTION_NUM};
};
template<typename CC>//csid content (std::stringやcsidContentを継承した型を使える)
class tableCsids:noMovCopyable,
	public tableCsidsKeyword
{
	//csidを記録・管理・抽出するtable
	private:
	static const CC emptyContent;
	const std::string name;
	const std::string& getSelfName()const{return name;}
	class aCsid {
		private:
		csidType csid;
		struct aVar{//var=variation
			CC content;
			std::vector<infoType> FLInfos;
			int optCont[INT_OPTION_NUM]={};
			int totalCont()const{return FLInfos.size();}
			template<class CCA,class ITA>//CCA=csid content arg ITA=info type arg
			aVar(CCA &&contenta,ITA &&infoa, const option opta);
			std::string getFLInfosStr()const{return infoTypeBase::getStrFrom(FLInfos);}
			void print() const;
		};
		std::vector<aVar> vars;
		
		const tableCsids &parent;
		typename decltype(vars)::iterator writev;
		bool isWritev;//https://teratail.com/questions/25576
		bool isWritevAndCheck(){return isWritev=(vars.begin()<=writev && writev<vars.end());}
		template<tableCsidsKeyword::option OPT> static bool isOnly(const aVar&);
		template<tableCsidsKeyword::option OPT> static bool isOpt(const aVar&);
		
		public:
		template<class CCA,class ITA>
		aCsid(const tableCsids &parenta,const csidType &csida, CCA &&contenta, ITA &&infoa, const option opta);
		template<class CCA,class ITA>
		void addv(CCA &&contenta, ITA &&infoa, const option opta);
		
		void selectWritev();
		const CC& getWriteContent() const;
		const csidType& getCsid()const{return csid;}
		const std::string getSelfName() const;
		std::string getFLInfosStr()const;
		void print() const;
	};
	std::vector<aCsid> csids;
	constexpr static int FINDER_NONE=-1;
	flexibleCsidFinder<int> csidsFinder{FINDER_NONE};
	public:
	tableCsids(const std::string &namea);
	template<class CCA,class ITA>
	void addc(const csidType &csida,CCA &&contenta,ITA &&infoa,const option opta=NONE);
	void selectWritevs();
	const CC& getWriteContent(const csidType &csida)const;
	bool isThereCsid(const csidType &csida)const;
	void print() const;
};

template<typename CC>
inline const std::string tableCsids<CC>::aCsid::getSelfName() const{
	return parent.getSelfName()+"CSID \""+csid+"\"";
}
template<typename CC>
inline const CC& tableCsids<CC>::getWriteContent(const csidType &csida)const{
	return csids[csidsFinder.find(csida)].getWriteContent();
}
struct tablesType:noMovCopyable{
	tableCsids<csidContentDetail> part{"part"};
	tableCsids<csidContent> line{"line"};
	void selectWritevs();
	void print();
};
class csidNamespaceTable{
	private:
	std::list<tablesType> ttable;
	constexpr static tablesType *FINDER_NULL=nullptr;
	flexibleCsidFinder<tablesType*> ttableFinder{FINDER_NULL};
	public:
	tablesType& refTable(const csidType&);
	tablesType& operator()(const csidType&);
	void selectWritevs();
	void print();
};
inline tablesType& csidNamespaceTable::operator()(const csidType &csid){
	return refTable(csid);
}


//////////////////////// filesBackupper
class filesBackupper {
	//ファイルをバックアップするためのやつ
	private:
	static fs::path toCurrentGenerator(fs::path fromabs);
	const fs::path fromCurrent,toCurrent;
	void copyWithCreateDir(const fs::path &froma,const fs::path &toa);
	public:
	filesBackupper(const fs::path &froma,const fs::path &toa);
	filesBackupper(const fs::path &froma);
	void backup(const fs::path &backupFilea);
};


//////////////////////// table <-> lineReader
class tableLineBase{
	protected:
	static const csidType globalCsid;
};
class tableLineReader:virtual public tableLineBase{
	//lineReaderからtableにデータを書き込む
	private:
	const csidType &mother;
	csidType name;
	csidNamespaceTable &table;
	lineReader &line;
	const infoType info;
	tableCsidsKeyword::option optConv(csidLineReaderRapChecker::option);
	doubleInfo getDoubleInfo();
	csidContentDetail readACsid(csidReader4Read&);
	public:
	void read();
	tableLineReader(const csidType&,csidNamespaceTable&,lineReader&,const infoTypeCastable&);
	template<class ITA>
	tableLineReader(const csidType&,csidNamespaceTable&,lineReader&,ITA&&);
};
inline doubleInfo tableLineReader::getDoubleInfo(){
	return doubleInfo(info,infoTypeCast(line.getNow()));
}
class tableLineWriter:virtual public tableLineBase{
	//tableからlineWriterにデータを書き込む
	private:
	const csidType &mother;
	csidType name;
	csidNamespaceTable &table;
	lineWriter &line;
	void writeACsid(csidReader4Write&);
	public:
	void write();
	tableLineWriter(const csidType&,csidNamespaceTable&,lineWriter&);
};
class tableLineRW:
	public tableLineReader,
	public tableLineWriter
{
	public:
	tableLineRW(const csidType&,csidNamespaceTable&,lineRW&,const infoTypeCastable&);
};


//////////////////////// targetDirFiles
class targetDirFiles:noMovCopyable{
	//ここより上にあったclassなどをまとめてcodeSyncとしての仕事を行う最も高級な奴
	private:
	static const csidType globalCsid;
	bool isTagExtension(const fs::path);
	void addf(const fs::directory_entry&);
	fs::path shapingTagDir(const fs::path&);
	void addInternalCsid(const csidType&,const std::string&);
	void addInternalCsidsIfDef();
	//void sortf();
	
	const fs::path tagDir;
	const std::vector<fs::path> tagExtensions;
	
	//tablesType table;
	csidNamespaceTable table;
	filesBackupper backupper;
	
	class aFile:noMovCopyable{
		private:
		lineFileReader fileRW;
		const fs::directory_entry fsEntry;
		const fs::file_time_type initTime;
		const std::string relPathStr;
		const fileInfo info;
		const csidType selfCsid;
		tableLineRW tlrw;
		
		public:
		aFile(csidNamespaceTable&,const fs::directory_entry&,const fs::path&);
		void load(){tlrw.read();}
		void write(){tlrw.write();}
		void backupBy(filesBackupper &backupper);
		std::string getPathStr()const{return relPathStr;}
		std::string getSelfName()const{return "File Path \""+getPathStr()+"\"";}
		
		fileInfo getFileInfo()const{return info;}
		bool isUpdated()const;
	};
	std::list<aFile> files;
	public:
	targetDirFiles(const fs::path &tagDira,const std::vector<fs::path> &tagExtensionsA);

	void load();
	void write();
	void backup();
	void printFiles2em() const;
	void printTable(){table.print();}
	
};
inline void targetDirFiles::aFile::backupBy(filesBackupper &backupper){
	backupper.backup(fsEntry.path());
}
inline bool targetDirFiles::aFile::isUpdated()const{
	return initTime<fs::last_write_time(fsEntry.path());
}
}//namespace codeSync

#endif