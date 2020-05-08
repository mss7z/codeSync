#include "codeSync.hpp"

namespace codeSync{
	
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

}//namespace codeSync