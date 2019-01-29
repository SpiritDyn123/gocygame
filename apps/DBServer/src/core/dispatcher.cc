/*
 * Dispatcher.cc
 *
 *  Created on: 2017年2月9日
 *      Author: antigloss
 */

#include "dispatcher.h"

void MessageDispatcher::PrintSupportedProtos()
{
	INFO_LOG("==============================================================");
	INFO_LOG("=                      Supported Protos                      =");
	INFO_LOG("==============================================================");
	for (uint32_t i = 0; i != kMaxHandlerNum; ++i) {
		if (handlers_[i]) {
			INFO_LOG("ProtoID=%u\t\tReq=%s Rsp=%s", i,
						handlers_[i]->GetReqName().c_str(),
						handlers_[i]->GetRspName().c_str());
		}
	}
}
