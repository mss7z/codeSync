#include "codeSync.hpp"

namespace codeSync{
//////////////////////// errorManager
errorManager em;

errorManager::css::css (errorManager *parenta):
	std::stringstream(),parentp(parenta)
{
	parentp->cssLocker.push_back(this);
	//DBGOUT("constract css:"<<parentp->cssLocker.size()<<std::endl);
}
void errorManager::css::resetSS(){
	str("");
	clear();
}
errorManager::css::~css (){
	const std::vector<css*>::iterator itr=std::find(parentp->cssLocker.begin(),parentp->cssLocker.end(),this);
	if(itr!=parentp->cssLocker.end()){
		parentp->cssLocker.erase(itr);
	}else{
		DBGOUT("割と深刻なエラー 自分を見失った @errorManager::css::~css"<<std::endl);
	}
}
std::ostream& operator<<(std::ostream &stream,const errorManager &manager){
	manager.info.writeTo(stream,"INFOMATION");
	manager.err.writeTo(stream,"ERROR");
	manager.warn.writeTo(stream,"WARNING");
	manager.chk.writeTo(stream,"PLEASE CHECK");
	return stream;
}

//////////////////////// timeLib
template<typename TP>
timeLib<TP>::timeLib(const TP tpa):
	//ここで変換するが処理時間などの影響で精度落ちると思われる
	tp((tpa - decltype(tpa)::clock::now())+ch::system_clock::now()),
	//  ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~    ^~~~~~~~~~~~~~~~~~~~~~~~
	//  ch::duration(現在時刻との差)         ch::time_point<system_clock>
	//C++20で可能になると思われる変換
	//sys_time t=clock_cast<system_clock>(tp);
	tt(ch::system_clock::to_time_t(tp)),
	tmt(localtimeGen(tt))
{}
template<typename TP>
std::string timeLib<TP>::getStr() const{
	std::stringstream s;//ここら辺は、Cライクに書いたほうが圧倒的に高速でよき？
	s<<*this<<std::flush;
	return s.str();
}
template<typename TP>
std::string timeLib<TP>::getStrOnlyNum() const{
	std::stringstream s;//ここら辺は、Cライクに書いたほうが圧倒的に高速でよき？
	s<<std::put_time(&tmt,"%Y%m%d%H%M%S")<<std::flush;
	return s.str();
}
template<typename TPA>
std::ostream& operator<<(std::ostream &stream,const timeLib<TPA> &tl){
	stream<<std::put_time(&(tl.tmt),"%Y/%m/%d(%a) %H:%M:%S");
	return stream;
}

//////////////////////// infoType
template<class IT>
std::string infoTypeBase::getStrFrom(const std::vector<IT> &v){
	typename std::remove_reference<decltype(v)>::type::const_iterator itr=v.cbegin(),end=v.cend();
	if(itr==end)return "";
	std::string s="\n\t\t"+itr->getStr();
	for(itr++;itr!=end;itr++){
		s+="\n\t〃\t"+itr->getStr();
	}
	return s;
}
infoType::infoType(const infoTypeBase *p):
	itp(p)
{
	//DBGOUTLN("ポインタモードでinfoTypeが生成"<<SVAL(itp->getStr()));
}
infoType::infoType(const infoTypeBase &d):
	itp(std::shared_ptr<const infoTypeBase>{},&d)
{
	//DBGOUTLN("参照モードでinfoTypeが生成"<<SVAL(itp->getStr()));
}
std::vector<fileInfo::fileInfoBody> fileInfo::bodys;

//////////////////////// streamLocker
auto streamLockerClass::close(std::fstream *tag){
	std::vector<std::fstream*>::iterator tagItr=std::find(fstreams.begin(),fstreams.end(),tag);
	if(tagItr!=fstreams.end()){
		(*tag).close();
		delete tag;
		*tagItr=nullptr;
	}
	return tagItr;
}
std::fstream* streamLockerClass::getP(fs::path path){
	std::fstream *fp=nullptr;
	try{fp=new std::fstream(path);}
	catch(std::bad_alloc &e){
		std::cerr<<"メモリが不足しているようです\nfstreamの動的確保に失敗\n"<<e.what()<<std::endl;
		std::exit(EXIT_FAILURE);
	}
	fstreams.push_back(fp);
	return fp;
}
streamLockerClass::~streamLockerClass(){
	for(const auto &i : fstreams){
		close(i);
	}
}
streamLockerClass streamLocker;

//////////////////////// LINE!!!
lineStreamRW::lineStreamRW(std::iostream *stpa):
	stp(stpa)
{}
const lineType& lineStreamRW::getNext(){
	if(isLastLine){
		li.unavailable();
		return li;
	}
	std::getline((*stp),li.lineStr);
	li.lineNum++;
	isLastLine=isStreamEOS();
	return li;
}
const lineType& lineStreamRW::getNow(){
	return li;
}
void lineStreamRW::addNext(const std::string &s){
	DBGOUTLN("addNext()は次の文字列を追加します: "<<s);
	if(isFirstLine){
		(*stp)<<s<<std::flush;
		isFirstLine=false;
	}else{
		(*stp)<<"\n"<<s<<std::flush;
	}
}
	
void lineStreamRW::resetSt(){
	stp->clear();
	stp->seekg(0,std::ios_base::beg);
	//li.available();
	isLastLine=isStreamEOS();
	isFirstLine=true;
}
void lineStreamRW::gotoLine(int tag){
	//現在使用されていない関数
	DBGOUT(li.lineNum<<"から"<<tag);
	if(tag<=li.lineNum){
		resetBeforeRead();
		DBGOUT(" 初期化して"<<li.lineNum<<"から"<<tag);
		if(tag==0 && li.lineNum==0)return;
	}
	DBGOUT(std::endl);
	assert(tag>li.lineNum);
	while(true){
		DBGOUTLN("gotoLine At"<<li.lineNum);
		if(li.lineNum==tag-1){
			DBGOUTLN("lineNum="<<li.lineNum<<" に到達したため、gotoLineは終了します @gotoLine");
			return;
		}
		if(!(getNext().isAvailable())){
			throw std::logic_error("目標行数が見つかりません @lineStreamRW::gotoLine");
		}
	}
}

void lineFileRW::chkOpened(){
	if(!(*f))throw fileError(getSelfName()+"を開けない");
}
lineFileRW::lineFileRW(fs::path fname):
	lineStreamRW(f=streamLocker.getP(fname)),path(fname)
{
	chkOpened();
}
void lineFileRW::reopenAs(std::ios_base::openmode mode){
	f->close();
	f->open(path,mode);
	chkOpened();
}
void lineFileRW::resetBeforeWrite(){
	resetSt();
	delContent();
}

lineSeekReader::lineSeekReader(lineStreamRW &lordLLa):
	lordLL(lordLLa)
{}
lineSeekReader::lineSeekReader(lineSeekReader &lsl):
	lineSeekReader(lsl.getLordLL())
{}
const lineType& lineSeekReader::getNext(){
	if(lordLL.getLoadedLineNum()>=endLineNum){
		return emptyLi;
	}
	return lordLL.getNext();
}
void lineSeekReader::resetBeforeRead(){
	captureMark();
	lordLL.gotoLine(startLineNum);
}

void lineSeekReader::captureStart(int offset){
	startLineNum=lordLL.getLoadedLineNum()+offset;
}
void lineSeekReader::captureEnd(int offset){
	endLineNum=lordLL.getLoadedLineNum()+offset;
}
void lineSeekReader::captureMark(int offset){
	markLineNum=lordLL.getLoadedLineNum()+offset;
}
void lineSeekReader::restoreMark(){
	lordLL.gotoLine(markLineNum);
}

lineStringReader::lineStringReader(lineStreamRW &mother):
	lineStreamRW(&ss)
{
	//https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
	//http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
	//いろいろあるみたいだがとりあえず簡単な奴
	mother.resetSt();
	ss<<mother.getRdbuf();
	print();
}

lineStringReader::lineStringReader(const std::string &s):
	lineStreamRW(&ss),ss(s)
{}
void lineStringReader::resetBeforeWrite(){
	resetSt();
	ss.str("");
}

//////////////////////// csidType
template<typename T>
csidDictHash<T>::csidDictHash(const T &nonea):
	hashTableEndP(hashTable+hashTableSize),none(nonea)
{
	for(T *p=hashTable;p<hashTableEndP;p++){
		*p=none;
	}
	tableP=hashTable;
}
csidDictHash<csidType::indexType> csidType::hash{dict.max_size()};

std::vector<std::string> csidType::dict={""};
const csidType::indexType csidType::emptyIndex=findSame("");
const csidType csidType::emptyCsid{};
csidType::indexType csidType::findSame(const std::string &s){
	indexType cdindex=hash.getCandidate(s);
	while(true){
		if(cdindex==hash.none){
			//DBGOUTLN("hash=NONE!");
			return emptyIndex;
		}else if(dict[cdindex]==s){
			//DBGOUTLN("hash=FOUND!!!! ハッシュに登録されていました");
			return cdindex;
		}else{
			//DBGOUTLN("hash=BAD ハッシュが衝突しました");
			cdindex=hash.nextCandidate();
		}
		if(hash.isFull()){
			//DBGOUTLN("hash=FULL! ハッシュが満杯です");//万が一ハッシュがいっぱいになったら頑張ってひとつずつ探す
			const std::vector<std::string>::const_iterator begin=dict.begin(),end=dict.end();
			const std::vector<std::string>::const_iterator findItr=std::find(begin,end,s);
			if(findItr==end){
				return emptyIndex;
			}
			return std::distance(begin,findItr);
		}
	}
}
csidType::indexType csidType::findSameElseAdd2Dict(const std::string &s){
	//DBGOUTLN("search "<<s);
	if(const indexType cdindex=findSame(s);cdindex==emptyIndex){
		const indexType newIndex=dict.size();
		hash.setCandidate(newIndex);
		dict.push_back(s);
		return newIndex;
	}else{
		return cdindex;
	}
}
csidType::csidType(const indexType &v):
	index(v){}
csidType csidType::getIfFind(const std::string &s){
	if(const indexType cdindex=findSame(s);cdindex==emptyIndex){
		return emptyCsid;
	}else{
		return csidType{cdindex};
	}
}

csidType::csidType():
	index(emptyIndex){}
csidType::csidType(const std::string &s):
	index(findSameElseAdd2Dict(s)){}
csidType::csidType(const csidType &v):
	index(v.getIndex()){}
	
std::ostream& operator<<(std::ostream &stream,const csidType &v){
	stream<<v.getStr();
	return stream;
}
std::istream& operator>>(std::istream &stream,csidType &v){
	std::string s;
	stream>>s;
	v=s;
	return stream;
}
csidType::operator std::string()const{
	return getStr();
}
csidType::operator csidType::indexType()const{
	return static_cast<csidType::indexType>(getIndex());
}

template<typename T>
csidFinderBase<T>::csidFinderBase(const tableST sizea,const T &nonea):
	table(sizea,nonea),none(nonea)
{}
template<typename T>
void csidFinderBase<T>::unset(const csidType &toa){
	table.at(static_cast<tableST>(toa))=none;
}
template<typename T>
flexibleCsidFinder<T>::flexibleCsidFinder(const T &nonea):
	csidFinderBase<T>(csidType::getCsidTotal()+tableAddSizeWhenResize,nonea)
{}
template<typename T>
void flexibleCsidFinder<T>::set(const csidType &toa,const T &vala){
	const auto toIndex=static_cast<tableST>(toa);
	if(this->table.size()>toIndex){
		this->table[toIndex]=vala;
	}else{
		this->table.resize(toIndex+tableAddSizeWhenResize,this->none);
		this->table[toIndex]=vala;
	}
}
template<typename T>
auto flexibleCsidFinder<T>::find(const csidType &toa)const
	->typename std::conditional<std::is_same<T,bool>::value,bool,const T&>::type
{
	const auto toIndex=static_cast<tableST>(toa);
	if(this->table.size()>toIndex){
		return this->table[static_cast<tableST>(toa)];
	}else{
		return this->none;
	}
}
template<typename T>
constCsidFinder<T>::constCsidFinder(const T &nonea):
	csidFinderBase<T>(csidType::getCsidTotal(),nonea)
{}

//////////////////////// csidReader(RW)
bool csidLineReader::isPracticableNextLine(){
	line=ll->getNext();
	return line.isAvailable();
}
bool csidLineReader::isPracticableCsidSeparaterSS(){
	const std::string::size_type startIndex=getLineStr().find(csidTag);
	if(startIndex==std::string::npos){
		return false;
	}
	csidSeparaterSS.clear();//一応フラグ初期化
	csidSeparaterSS.str(getLineStr().substr(startIndex+csidTagSize));
	return true;
}
csidLineReader::status csidLineReader::getCmdFromCsidSeparaterSS(){
	std::string cmdStr;
	csidSeparaterSS>>cmdStr;
	str2lower(cmdStr);
	if(isNoneCmdPart){
		const status cmd=getCmdSwitchWhenNone(cmdStr);
		if(cmd==NONE_END){
			isNoneCmdPart=false;
		}else if(cmd==EGG){
			std::cerr<<"大変だ！ Documentの中に卵があった！"<<std::endl;
		}
		return NONEC;
	}else{
		cns=getCsidNamespaceSwitch(cmdStr);
		if(cns==UNKNOWN_CNS){
			cns=GLOBAL;
		}else{
			csidSeparaterSS>>cmdStr;
		}
		const status cmd=getCmdSwitchWhenEnable(cmdStr);
		if(cmd==NONE_START){
			isNoneCmdPart=true;
			return NONEC;
		}else if(cmd==NONE_END){
			throw fileError("nonestartしていないのにnoneendした");
		}else if(cmd==UNKNOWN_STS){
			throw fileError("\""+cmdStr+"\" は不明な命令");
		}
		return cmd;
	}
}
csidLineReader::status csidLineReader::getCmdSwitchWhenNone(const std::string &cmd)noexcept{
	if(cmd=="noneend"){
		return NONE_END;
	}else if(cmd=="egg"){
		return EGG;
	}else{
		return UNKNOWN_STS;
	}
}

csidLineReader::status csidLineReader::getCmdSwitchWhenEnable(const std::string &cmd)noexcept{
	if(cmd=="line"){
		opt=NONE;
		return LINE;
	}else if(cmd=="masterline"){
		opt=MASTER;
		return LINE;
	}else if(cmd=="start"){
		opt=NONE;
		return START;
	}else if(cmd=="master"){
		opt=MASTER;
		return START;
	}else if(cmd=="unmaster"){
		opt=UNMASTER;
		return START;
	}else if(cmd=="end"){
		return END;
	}else if(cmd=="none"){
		return ORDINARILY;
	}else if(cmd=="nonestart"){
		return NONE_START;
	}else if(cmd=="noneend"){
		return NONE_END;
	}else{
		return UNKNOWN_STS;
	}
}
csidLineReader::csidNamespace csidLineReader::getCsidNamespaceSwitch(const std::string &cns)noexcept{
	if(cns=="file"){
		return FILE;
	}else if(cns=="name"){
		return NAME;
	}else if(cns=="global"){
		return GLOBAL;
	}else{
		return UNKNOWN_CNS;
	}
}

void csidLineReader::loadCsidFromCsidSeparaterSS(){
	csid.clear();
	csidSeparaterSS>>std::ws;
	if(!csidSeparaterSS.good()){
		throw fileError("CSIDが虚無");
	}
	if(csidSeparaterSS.peek()=='\"'){
		std::string s;
		csidSeparaterSS.get();
		decltype(csidSeparaterSS)::int_type c;
		while(true){
			c=csidSeparaterSS.get();
			if(!csidSeparaterSS.good())throw fileError("\' \" \'でcsidが閉じられていない");
			if(c=='\"')break;
			s+=c;
		}
		csid=s;
	}else{
		csidSeparaterSS>>csid;
	}
	if(csid.empty()){
		throw fileError("CSIDが虚無 (\"\")");
	}
}

csidLineReader::csidLineReader(lineReader *p):
	ll(p)
{}
csidLineReader::csidLineReader(lineReader &r):
	csidLineReader(&r)
{}

analyzedLine csidLineReader::getAnalyzedLine(){
	return analyzedLine{cmd,csid,opt,cns};
}

void csidLineReader::resetBeforeNextLine(){
	ll->resetBeforeRead();
}

csidLineReader::status csidLineReader::nextLine(){
	if(!isPracticableNextLine()){
		DBGOUTLN(SVAL(line.lineNum));
		return EOS;
	}
	if(isPracticableCsidSeparaterSS()){
		cmd=getCmdFromCsidSeparaterSS();
		if(cmd==LINE){
			loadCsidFromCsidSeparaterSS();
			return LINE;
		}else if(cmd==START){
			loadCsidFromCsidSeparaterSS();
			return START;
		}else if(cmd==END){
			return END;
		}
	}
	cmd=ORDINARILY;
	return ORDINARILY;
}


std::string_view csidLineReader::getLineMain(){
	std::string_view s{line.lineStr};
	decltype(s)::size_type i;
	for(i=0;i<s.size();i++){
		if(!(s[i]==' ' || s[i]=='\t')){
			s.remove_prefix(i);
			break;
		}
	}
	return s;
}

std::string csidLineReaderRapChecker::getDependentStr()const{
	decltype(inStack)::const_iterator itr=inStack.begin();
	const decltype(inStack)::const_iterator end=inStack.end();
	if(itr==end)throw std::logic_error("関係の表示を試みるが、Stackが空");
	std::string s("依存関係 : "+itr->getStrQuote());
	for(itr++;itr!=end;itr++){
		s+=" <- "+itr->getStrQuote();
	}
	return s;
}
csidLineReaderRapChecker::status csidLineReaderRapChecker::nextLine(){
	const status sts=csidLineReader::nextLine();
	switch(sts){
		case START:
		inStack.push_back(getCsid());
		if(finder.find(getCsid())){
			throw fileError("壊れた依存関係を発見しました: \n"+getDependentStr()+'\n'+getCsid().getStrCsid()+"は、内部で自身を必要としています");
		}
		finder.set(getCsid());
		if(inStack.size()>maxInLevel){
			throw fileError("ｱｯ このネスト、深い!!!!!\nネストが想定より深くスタックオーバーフローする恐れがあるため、このファイルは処理できません");
		}
		break;
		case END:
		if(inStack.size()==0)throw fileError("CSIDを閉じる回数が多い");
		finder.unset(inStack.back());
		inStack.pop_back();
		break;
		case EOS:
		if(inStack.size()!=0)throw fileError("CSIDが閉じられないままファイルが終了\nもしかして、"+inStack.back().getStrCsid()+" の閉じ忘れ?");
		break;
		default:break;
	}
	return sts;
}


//////////////////////// csidContent
int csidContent::addToStrsWithNum(const std::string &s){
	const int num=strs.size();
	addToStrs(s);
	return num;
}
csidContent::csidContent(csidContent &&c)noexcept:
	strs(std::move(c.strs)){}
csidContent::csidContent(const csidContent &c)noexcept:
	strs(c.strs){}
csidContent& csidContent::operator=(csidContent &&c)noexcept{
	strs=std::move(c.strs);
	return *this;
}

std::ostream& operator<<(std::ostream& stream,const csidContent &content){
	const std::vector<std::string>::const_iterator end=content.strs.cend();
	std::vector<std::string>::const_iterator itr=content.strs.cbegin();
	if(itr==end)return stream;
	stream<<"size="<<content.strs.size()<<"\n";
	stream<<*itr;
	for(itr++;itr!=end;itr++){
		stream<<'\n'<<*itr;
	}
	return stream;
}


csidContentLineWriter::csidContentLineWriter(const std::string &s){
	assert(s.find('\n')==std::string::npos);
	addToStrs(s);
}
csidContentLineReader::csidContentLineReader(const csidContent &lordCCa):
	lordCC(lordCCa)
{
	assert(lordCC.refStrs().size()==1);
}


lineCsidContentReader::lineCsidContentReader(const csidContent &lordCCa):
	lordCC(lordCCa)
{
	
}


csidContentDetail::csidContentDetail(csidContentDetail &&v)noexcept:
	csidContent(std::move(v)),infos(std::move(v.infos))
{}
csidContentDetail::csidContentDetail(const csidContentDetail &v)noexcept:
	csidContent(v),infos(v.infos)
{}

void csidContentDetailWriter::addLine(const std::string &s,const analyzedLine al){
	assert(al.sts!=ORDINARILY);
	const int refNum=addToStrsWithNum(s);
	infos.emplace_back(refNum,al);
}


const analyzedLine csidContentDetailReader::eosAnalyzed{EOS,csidType::emptyCsid,UNKNOWN_OPT,UNKNOWN_CNS};
const analyzedLine csidContentDetailReader::ordinaryAnalyzed{ORDINARILY,csidType::emptyCsid,UNKNOWN_OPT,UNKNOWN_CNS};
const std::string csidContentDetailReader::emptyStr{};

csidContentDetailReader::csidContentDetailReader(const csidContentDetail &lordCCDa):
	lordCCD(lordCCDa){}
void csidContentDetailReader::reset(){
	infosItr=lordCCD.refInfos().begin();
	isInfosItr=infosItr!=lordCCD.refInfos().end();
	strsNumCont=0;
}
std::pair<const std::string&,const analyzedLine&> csidContentDetailReader::getLine(){
	if(!(strsNumCont<int(lordCCD.refStrs().size()))){
		return {emptyStr,eosAnalyzed};
	}
	if(isInfosItr){
		if((*infosItr).refNum==strsNumCont){
			//DBGOUTLN("通常の行でないことを発見しました、行のstsは"<<(*infosItr).analyzed.sts);
			isInfosItr=(infosItr+1)!=lordCCD.refInfos().end();
			return {lordCCD.refStrs().at(strsNumCont++),(*(infosItr++)).analyzed};
		}else{
			//DBGOUTLN("通常の行のためORDINARYです");
			return {lordCCD.refStrs().at(strsNumCont++),ordinaryAnalyzed};
		}
	}else{
		//DBGOUTLN("isInfosItr==falseで、残りは通常の行しかないと思われます。通常の行のためORDINARYです");
		return {lordCCD.refStrs().at(strsNumCont++),ordinaryAnalyzed};
	}
}

csidCsidContentDetailReader::csidCsidContentDetailReader( csidContentDetailReader &lordCCDRa):
	lordCCDR(lordCCDRa)
{}



const std::string& csidCsidContentDetailReader::getLineStr(){
	assert(lineStrP!=nullptr);
	return *lineStrP;
}
const csidType& csidCsidContentDetailReader::getCsid(){
	assert(analyzedP!=nullptr);
	return analyzedP->csid;
}
void csidCsidContentDetailReader::resetBeforeNextLine(){
	lordCCDR.reset();
	analyzedP=nullptr;
	lineStrP=nullptr;
}
csidCsidContentDetailReader::status csidCsidContentDetailReader::nextLine(){
	auto[refLineStr,refAnalyzed]=lordCCDR.getLine();
	lineStrP=&refLineStr;
	analyzedP=&refAnalyzed;
	return refAnalyzed.sts;
}
csidCsidContentDetailReader::csidNamespace csidCsidContentDetailReader::getCsidNamespace(){
	assert(analyzedP!=nullptr);
	return analyzedP->cns;
}
std::string_view csidCsidContentDetailReader::getLineIndent(){
	std::string_view s{getLineStr()};
	decltype(s)::size_type i;
	for(i=0;i<s.size();i++){
		if(!(s[i]==' ' || s[i]=='\t')){
			break;
		}
	}
	s.remove_suffix(s.size()-i);
	return s;
}

//////////////////////// TABLE!!!!!
template<typename CC>
const CC tableCsids<CC>::emptyContent;

template<typename CC>
template<class CCA,class ITA>
tableCsids<CC>::aCsid::aVar::aVar(CCA &&contenta,ITA &&infoa, const option opta):
	content(std::forward<CCA>(contenta))
{
	optCont[opta]++;
	FLInfos.emplace_back(std::forward<ITA>(infoa));
}
template<typename CC>
void tableCsids<CC>::aCsid::aVar::print() const{
	std::cout<<"aVar "<<" -----------\n"<<content<<"<END"<<std::endl;
}
template<typename CC>
template<tableCsidsKeyword::option OPT>
bool tableCsids<CC>::aCsid::isOnly(const aVar &vara){
	for(int i=0;i<INT_OPTION_NUM;i++){
		if(i==OPT)continue;
		if(vara.optCont[i]>0)return false;
	}
	return true;
}
template<typename CC>
template<tableCsidsKeyword::option OPT>
bool tableCsids<CC>::aCsid::isOpt(const aVar &vara){
	return vara.optCont[OPT]>0;
}
template<typename CC>
template<class CCA,class ITA>
tableCsids<CC>::aCsid::aCsid(const tableCsids &parenta,const csidType &csida, CCA &&contenta, ITA &&infoa, const option opta):
	parent(parenta)
{
	csid=csida;
	vars.emplace_back(std::forward<CCA>(contenta),std::forward<ITA>(infoa),opta);
}
template<typename CC>
template<class CCA,class ITA>
void tableCsids<CC>::aCsid::addv(CCA &&contenta,ITA &&infoa, const option opta){
	typename std::vector<aVar>::iterator varsAnsI = std::find_if(vars.begin(),vars.end(),[&](const aVar &varsI){
		return (varsI.content==contenta);
	});
	if(varsAnsI==vars.end()){
		vars.emplace_back(std::forward<CCA>(contenta),std::forward<ITA>(infoa),opta);
	}else{
		((varsAnsI->optCont)[opta])++;
		varsAnsI->FLInfos.emplace_back(std::forward<ITA>(infoa));
	}
}


template<typename CC>
void tableCsids<CC>::aCsid::selectWritev(){
	auto isCont1=[](const aVar &i){return i.totalCont()==1;};
	auto isEmpty=[](const aVar &i){return i.content.isEmpty();};
	auto isEnableVar=[&](const aVar &i){return (!isEmpty(i))&&(!isOnly<UNMASTER>(i));};
	auto isEnableCont1=[&](const aVar &i){return isCont1(i)&&isEnableVar(i);};
	auto contCsidAppearance=[&](){
		int cont=0;
		for(const auto &i:vars){cont+=i.totalCont();}
		return cont;
	};
	const typename std::vector<aVar>::iterator begin=vars.begin(),end=vars.end();
	const int emptyVarCont=std::count_if(begin,end,isEmpty);
	const int enableCont1varCont=std::count_if(begin,end,isEnableCont1);
	const int varCont=vars.size();
	const int csidAppearanceCont=contCsidAppearance();
	const int enableVarCont=std::count_if(begin,end,isEnableVar);
	const int masterVarCont=std::count_if(begin,end,isOpt<MASTER>);
	const int unmasterVarCont=std::count_if(begin,end,isOpt<UNMASTER>);
	const int internalVarCont=std::count_if(begin,end,isOpt<INTERNAL>);
	//可読性悪すぎ
	if(internalVarCont>0){
		//プログラム内部で自動生成されるinternal optionがある場合
		if(internalVarCont!=1){
			throw std::logic_error("option=INTERNALのaVarが複数存在");
		}
		if(masterVarCont>0){
			em.warn<<getSelfName()<<"は、master指定がありますが、強制的に上書きされます"<<getFLInfosStr()<<std::endl;
		}
		writev=std::find_if(begin,end,isOpt<INTERNAL>);
	}else if(masterVarCont>0){
		//master指定のあるvariationがある場合
		if(masterVarCont==1){
			if(varCont>2){
				em.warn<<getSelfName()<<"は明示的にマスター指定されていますが、マスターを含めて"<<varCont<<"種類があり、masterで上書きされます"<<getFLInfosStr()<<std::endl;
			}
			writev=std::find_if(begin,end,isOpt<MASTER>);
			if(writev->optCont[MASTER]!=1){
				em.chk<<getSelfName()<<"は"<<writev->optCont[MASTER]<<"箇所で明示的にmaster指定されていますが、内容が同じため書き込みされます"<<getFLInfosStr()<<std::endl;
			}
		}else{
			em.err<<getSelfName()<<"には複数の明示的なマスター指定があります"<<getFLInfosStr()<<std::endl;
		}
	}else if(0==enableVarCont){
		//空白やunmaster指定などしかなく、有効なvariationがない場合
		if(csidAppearanceCont==1){
			if(emptyVarCont>0){
				em.chk<<"typo? "<<getSelfName()<<"は1箇所しかなく、空白です"<<getFLInfosStr()<<std::endl;
			}else{
				em.warn<<"typo? "<<getSelfName()<<"は1箇所しかなく、unmasterであるため、内容が削除されます"<<getFLInfosStr()<<std::endl;
			}
		}else{
			if(unmasterVarCont>0){
				em.warn<<getSelfName()<<"は"<<csidAppearanceCont<<"箇所ありますが、空白かunmasterで、unmasterは内容が削除されます"<<getFLInfosStr()<<std::endl;
			}else{
				em.chk<<getSelfName()<<"は"<<csidAppearanceCont<<"箇所ありますが、どれも空白です"<<getFLInfosStr()<<std::endl;
			}
		}
	}else if(1==enableVarCont){
		//有効なvariationが一つの場合
		if(csidAppearanceCont==1){
			em.chk<<"typo? "<<getSelfName()<<"は1箇所しかありません"<<getFLInfosStr()<<std::endl;
		}
		writev=std::find_if(begin,end,isEnableVar);
	}else if(enableCont1varCont==1 && enableVarCont<=2){
		//有効かつcontが1で、それが変更先と予想でき、variationのcontが2以下である時
		//(2より多いと正しいmasterの推論ができず、使用者の意図しない結果となる可能性がある)
		em.info<<getSelfName()<<"は変更が行われます"<<std::endl;
		writev=std::find_if(begin,end,isEnableCont1);
	}else{
		//以上のどれにもあてはまらず、推論できない場合
		em.err<<getSelfName()<<"は"<<varCont<<"種類があり、masterを特定できません"<<getFLInfosStr()<<std::endl;
	}
	DBGOUTLN("mileaa");
	if(isWritevAndCheck()){
		em.info<<getSelfName()<<"はこれがマスターになります"<<writev->getFLInfosStr()<<std::endl;
	}
	
	DBGOUT(getSelfName()<<"'s writev is "<<(isWritev?"true":"false")<<std::endl);
}
	
template<typename CC>
const CC& tableCsids<CC>::aCsid::getWriteContent() const{
	if(isWritev){
		return writev->content;
	}else{
		return emptyContent;
	}
}

template<typename CC>
std::string tableCsids<CC>::aCsid::getFLInfosStr()const{
	if(vars.size()==1){
		return vars[0].getFLInfosStr();
	}else{
		std::string s;
		int cont=0;
		for(const auto &i : vars){
			s+="\n"+std::to_string(++cont)+"種類目の種類"+i.getFLInfosStr();
		}
		return s;
	}
	
}
template<typename CC>
void tableCsids<CC>::aCsid::print() const{
	std::cout<<"aCsid name:\""<<csid<<"\" (No."<<int(csid.getIndex())<<")================"<<std::endl;
	for(const auto &i : vars){i.print();}
}
template<typename CC>
tableCsids<CC>::tableCsids(const std::string &namea):name(namea){}

template<typename CC>
template<class CCA,class ITA>
void tableCsids<CC>::addc(const csidType &csida, CCA &&contenta,ITA &&infoa, const option opta){
	const int index=csidsFinder.find(csida); 
	if(index==FINDER_NONE){
		csidsFinder.set(csida,static_cast<int>(csids.size()));
		csids.emplace_back(*this,csida,std::forward<CCA>(contenta),std::forward<ITA>(infoa),opta);
	}else{
		csids[index].addv(std::forward<CCA>(contenta),std::forward<ITA>(infoa),opta);
	}
}
template<typename CC>
void tableCsids<CC>::selectWritevs(){
	for(auto &i : csids){
		i.selectWritev();
	}
}


template<typename CC>
void tableCsids<CC>::print() const{
	for(const auto &i : csids){i.print();}
}
template<typename CC>
bool tableCsids<CC>::isThereCsid(const csidType &csida)const{
	return csidsFinder.find(csida)!=FINDER_NONE;
}

tablesType::tablesType(const std::string &s):
	part(s+"_part"),line(s+"_line"){}
void tablesType::selectWritevs(){
	part.selectWritevs();
	line.selectWritevs();
}
void tablesType::print(){
	part.print();
	line.print();
}
const csidType csidNamespaceTable::global=csidType::emptyCsid;
csidNamespaceTable::csidNamespaceTable(){
	ttableFinder.set(global,&ttable.emplace_back("global"));
}
tablesType& csidNamespaceTable::refTable(const csidType &csid){
	if(tablesType *p=ttableFinder.find(csid);p==FINDER_NULL){
		tablesType &ref=ttable.emplace_back(csid.getStrQuote());
		ttableFinder.set(csid,&ref);
		return ref;
	}else{
		return *p;
	}
}
void csidNamespaceTable::selectWritevs(){
	for(auto &i:ttable){
		i.selectWritevs();
	}
}
void csidNamespaceTable::print(){
	for(auto &i:ttable){
		i.print();
	}
}
//////////////////////// filesBackupper
fs::path filesBackupper::toCurrentGenerator(fs::path fromabs){
	assert(fromabs.filename()!="");
	
	const std::string fromDirName=fromabs.filename().string();
	const std::string backupParentDirName=fromDirName+"_CSBackups";
	const fs::path backupParentPath=fromabs.parent_path()/backupParentDirName;
	
	const std::string backupDirName=fromDirName+"_"+timeLib(ch::system_clock::now()).getStrOnlyNum();
	const fs::path backupPath=backupParentPath/backupDirName;
	if(!fs::create_directories(backupPath)){
		throw std::runtime_error("バックアップフォルダが作成できません");
	}
	em.info<<"バックアップフォルダ \""<<backupPath.string()<<"\" が作成されました"<<std::endl;
	return backupPath;
}

void filesBackupper::copyWithCreateDir(const fs::path &froma,const fs::path &toa){
	if(const fs::path pp=toa.parent_path();!fs::exists(pp)){
		fs::create_directories(pp);
	}
	fs::copy(froma,toa);
}
filesBackupper::filesBackupper(const fs::path &froma,const fs::path &toa):
	fromCurrent(fs::absolute(froma)),toCurrent(fs::absolute(toa))
{}
filesBackupper::filesBackupper(const fs::path &froma):
	fromCurrent(fs::absolute(froma)),toCurrent(toCurrentGenerator(fromCurrent))
{
	DBGOUT("filesBackupper::toCurrent :"<<toCurrent<<std::endl);
}

void filesBackupper::backup(const fs::path &backupFilea){
	const fs::path backupFileRelative=fs::relative(backupFilea,fromCurrent);
	const fs::path fromFile=fromCurrent/backupFileRelative;
	const fs::path toFile=toCurrent/backupFileRelative;
	copyWithCreateDir(fromFile,toFile);
	DBGOUT("backupFIlea:"<<backupFilea<<" filesBackupper::backup:"<<toFile<<std::endl);
}


//////////////////////// table <-> lineReader

tableCsidsKeyword::option tableLineReader::optConv(csidLineReaderRapChecker::option opta){
	using cr=csidLineReaderRapChecker;
	using to=tableCsidsKeyword;
	if(cr::NONE==opta){
		return to::NONE;
	}else if(cr::MASTER==opta){
		return to::MASTER;
	}else if(cr::UNMASTER==opta){
		return to::UNMASTER;
	}
	throw std::logic_error("予期しないoptionを受け取った @targetDirFiles::aFile::optConv");
}


csidContentDetail tableLineReader::readACsid(csidReader4Read &reader){
	csidContentDetailWriter inContent;
	const csidType initName=name;
	using cr=csidReader4Read;
	while(true){
		const cr::status sts=reader.nextLine();
		if(sts==cr::ORDINARILY){
			inContent.addLine(reader.getLineStr());
			continue;
		}else if(sts==cr::EOS || sts==cr::END){
			break;
		}
		const tableCsidsKeyword::option opt=optConv(reader.getOpt());
		const lineInfo lit=reader.getLineInfo();
		const csidType csid=reader.getCsid();
		const cr::csidNamespace cns=reader.getCsidNamespace();
		tablesType *tagTable;
		if(cns==cr::GLOBAL){
			tagTable=&globalT;
			name=csid;
		}else if(cns==cr::FILE){
			tagTable=&table(mother);
		}else if(cns==cr::NAME){
			tagTable=&table(name);
		}else{
			throw std::logic_error("予期しないcsidNamespace @csidContentDetail tableLineReader::readACsid");
		}
		if(sts==cr::START){
			inContent.addLine(reader.getLineStr(),reader.getAnalyzedLine());
			csidContentDetail content{readACsid(reader)};
			tagTable->part.addc(csid,content,infoTypeGen<doubleInfo>(info,infoTypeCast(lit)),opt);
			inContent.addLine(reader.getLineStr(),reader.getAnalyzedLine());
		}else if(sts==cr::LINE){
			inContent.addLine(reader.getLineStr(),reader.getAnalyzedLine());
			tagTable->line.addc(csid,csidContentLineWriter{std::string(reader.getLineMain())},infoTypeGen<doubleInfo>(info,infoTypeCast(lit)),opt);
		}else{
			throw std::logic_error("予期しないstatus @csidContentDetail tableLineReader::readACsid");
		}
		name=initName;
	}
	return std::move(inContent);//NRVOどこ行った？std::moveをつけないと普通にコピーされるのですが
}
void tableLineReader::read(){
	try{
		const tableCsidsKeyword::option opt=tableCsidsKeyword::INTERNAL;
		csidLineReaderRapChecker reader{line};
		reader.resetBeforeNextLine();
		csidContentDetail content{readACsid(reader)};
		globalT.part.addc(mother,content,info,opt);
	}catch(const fileError &s){
		em.err<<s<<getDoubleInfo().getStr()<<std::endl;
	}
}
tableLineReader::tableLineReader(const csidType &csida,csidNamespaceTable &tablea,lineReader &linea,const infoTypeCastable &infoa):
	tableLineBase(csida,tablea),line(linea),info(infoa){}
template<class ITA>
tableLineReader::tableLineReader(const csidType &csida,csidNamespaceTable &tablea,lineReader &linea,ITA &&infoa):
	tableLineBase(csida,tablea),line(linea),info(infoTypeCast(infoa)){}

void tableLineWriter::writeACsid(csidReader4Write &reader){
	reader.resetBeforeNextLine();
	const csidType initName=name;
	using cr=csidReader4Write;
	while(true){
		const cr::status sts=reader.nextLine();
		if(sts==cr::ORDINARILY || sts==cr::END){
			line.addNext(reader.getLineStr());
			continue;
		}else if(sts==cr::EOS){
			break;
		}
		const csidType csid=reader.getCsid();
		const cr::csidNamespace cns=reader.getCsidNamespace();
		const tablesType *tagTable;
		if(cns==cr::GLOBAL){
			tagTable=&globalT;
			name=csid;
		}else if(cns==cr::FILE){
			tagTable=&table(mother);
		}else if(cns==cr::NAME){
			tagTable=&table(name);
		}
		if(sts==cr::START){
			line.addNext(reader.getLineStr());
			const csidContentDetail &writeContent=tagTable->part.getWriteContent(csid);
			if(!writeContent.isEmpty()){
				csidContentDetailReader csidDetailRdr{writeContent};
				csidCsidContentDetailReader csidRdr{csidDetailRdr};
				writeACsid(csidRdr);
			}
		}else if(sts==cr::LINE){
			line.addNext(std::string(reader.getLineIndent())+csidContentLineReader{tagTable->line.getWriteContent(csid)}.getStr());
		}else{
			throw std::logic_error("予期しないstatus @targetDirFiles::aFile::loadACsid");
		}
		name=initName;
	}
	
}
void tableLineWriter::write(){
	try{
		line.resetBeforeWrite();
		const csidContentDetail &writeContent=globalT.part.getWriteContent(mother);
		if(writeContent.isEmpty()){
			throw std::logic_error("書き込み時に初期のファイルのcsidContentが空白");
		}
		csidContentDetailReader csidDetailRdr{writeContent};
		csidCsidContentDetailReader csidRdr{csidDetailRdr};
		writeACsid(csidRdr);
	}catch(const fileError &s){
		throw std::logic_error("内部文章の解析中に次の問題が発生しました\n"+s.getStr());
	}
}
tableLineWriter::tableLineWriter(const csidType &csida,csidNamespaceTable &tablea,lineWriter &linea):
	tableLineBase(csida,tablea),line(linea){}

tableLineRW::tableLineRW(const csidType &csida,csidNamespaceTable &tablea,lineRW &linea,const infoTypeCastable &infoa):
	tableLineBase(csida,tablea),
	tableLineReader(csida,tablea,linea,infoa),
	tableLineWriter(csida,tablea,linea)
{}


//////////////////////// targetDirFiles
bool targetDirFiles::isTagExtension(const fs::path ext){
	for(const auto& i : tagExtensions){
		if(ext==i)return true;
	}
	return false;
}
void targetDirFiles::addf(const fs::directory_entry& fsEntryA){
	try{
		files.emplace_back(table,fsEntryA,tagDir).read();
	}catch (const fileError &s){
		em.warn<<s<<" これにより、ファイル \""<<fsEntryA.path().string()<<" \"はスキップされます"<<std::endl;
		return;
	}
}
fs::path targetDirFiles::shapingTagDir(const fs::path &tag){
	if(!fs::exists(tag)){
		throw std::runtime_error("フォルダ \""+tag.string()+"\" が存在しません");
	}
	const fs::path ans=fs::absolute(tag);
	if(ans.filename()==""){
		return ans.parent_path();
	}else{
		return ans;
	}
}
csidType targetDirFiles::getIfThereIsCsidInGlobalTable(const std::string &s){
	csidType cdCsid=csidType::getIfFind(s);
	if(cdCsid==csidType::emptyCsid){
		return cdCsid;
	}else if(table(csidNamespaceTable::global).part.isThereCsid(cdCsid)){
		return cdCsid;
	}
	return csidType::emptyCsid;
}

void targetDirFiles::addInternalCsid(const csidType &csid,const std::string &str){
	lineStringReader lineRdr{str};
	tableLineReader{csid,table,lineRdr,stringInfo("INTERNAL "+csid.getStrCsid())}.read();
}
	
void targetDirFiles::addInternalCsidsIfDef(){
	if(const csidType csid=getIfThereIsCsidInGlobalTable("__codeSync_selfIntroduction");csid!=csidType::emptyCsid){
		const std::string s{"\
====================================================\n\
   ##  ##    This document was written with Code Sync.\n\
  ##  ##     \n\
 ##  ###### ## ####   Generated time:"+timeLib{ch::system_clock::now()}.getStr()+"\n\
  ##    ##  ## #  ##\n\
   ##  ##   ## ####\n\
===================================================="
		};
		addInternalCsid(csid,s);
	}
}

targetDirFiles::targetDirFiles(const fs::path &tagDira,const std::vector<fs::path> &tagExtensionsA):
	tagDir(shapingTagDir(tagDira)),tagExtensions(tagExtensionsA),backupper(tagDir)
{}

targetDirFiles::aFile::aFile(csidNamespaceTable &tablea,const fs::directory_entry &fsEntryA,const fs::path &tagDira):
	fileRW(fsEntryA.path()),
	fsEntry(fsEntryA),
	initTime(fsEntry.last_write_time()),
	relPathStr(fs::proximate(fsEntry.path(),tagDira.parent_path()).string()),
	info(fileInfo{getPathStr(),ftimeLib(initTime)}),
	selfCsid("__file_"+getPathStr()),
	tlrw(selfCsid,tablea,fileRW,info)
{}



void targetDirFiles::read(){
	fs::recursive_directory_iterator itr(tagDir);
	if(fs::begin(itr)==fs::end(itr)){
		throw std::runtime_error(tagDir.string()+std::string("は、空のフォルダです"));
	}
	for(const fs::directory_entry& i : itr){
		if(i.is_regular_file()){
			if(isTagExtension(i.path().extension())){
				addf(i);
			}
		}
	}
	printFiles2em();
	addInternalCsidsIfDef();
	table.selectWritevs();
}
void targetDirFiles::write(){
	for(const auto &i: files){
		if(i.isUpdated()){
			throw std::runtime_error(i.getSelfName()+"は、更新されています\nデータの予期しない上書きを防ぐため、書き換え前に停止しました\nどのファイルにも変更は行われていません");
		}
	}
	for(auto &i : files){
		i.write();
	}
}
void targetDirFiles::backup(){
	for(aFile &i: files){
		i.backupBy(backupper);
	}
}

void targetDirFiles::printFiles2em()const{
	em.info<<"次のファイルが読み込まれました";
	std::vector<fileInfo> v;
	for(const auto &i :files){
		v.push_back(i.getFileInfo());
	}
	em.info<<infoTypeBase::getStrFrom(v)<<std::endl;
}

}//namespace codeSync