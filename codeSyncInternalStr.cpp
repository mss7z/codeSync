#include "codeSync.hpp"

namespace codeSync{
	
void targetDirFiles::addInternalCsidsIfDef(){
	if(isThereCsidInGlobalTable("__codeSync_selfIntroductionEn")){
		const std::string s{"\
=================================================================\n\
   ##  ##    This document was written with Code Sync.\n\
  ##  ##     \n\
 ##  ###### ## ####   Generated time: "+timeLib{ch::system_clock::now()}.getStr()+"\n\
  ##    ##  ## #  ##\n\
   ##  ##   ## ####\n\
================================================================="
		};
		addInternalCsid(cdInternalCsid,s);
	}
	if(isThereCsidInGlobalTable("__codeSync_selfIntroductionJp")){
		const std::string s{"\
=================================================================\n\
   ##  ##    Code Syncによって同期されました.\n\
  ##  ##     \n\
 ##  ###### ## ####   生成時間: "+timeLib{ch::system_clock::now()}.getStr()+"\n\
  ##    ##  ## #  ##\n\
   ##  ##   ## ####\n\
================================================================="
		};
		addInternalCsid(cdInternalCsid,s);
	}
}

}//namespace codeSync