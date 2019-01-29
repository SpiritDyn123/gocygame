#ifndef DIGIMON_DISPATCHER_H_
#define DIGIMON_DISPATCHER_H_

#include <memory>
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <google/protobuf/message.h>
#include <serverbench/benchapi.hpp>

#include <Singleton.h>
#include <Proto/Proto.h>
#include "../proto/ErrCode.pb.h"
#include "../TimeUtils.h"

class AbsMessageHandlerHolder {
public:
	virtual ~AbsMessageHandlerHolder()
	{
	}

	// 返回-1直接关闭连接，0正常返回，1返回后关闭连接。一般只需要使用0和-1。
	virtual int Exec(const SSProtoHead& h, std::string& outbuf) = 0;
	virtual const std::string GetReqName() const = 0;
	virtual const std::string GetRspName() const = 0;
};

template<typename ClassType>
class MessageHandlerHolder : public AbsMessageHandlerHolder {
public:
	// 返回0表示成功，>0表示错误码
	using Method=ErrCodeType (ClassType::*)(const SSProtoHead& h, google::protobuf::Message* inMsg, google::protobuf::Message* outMsg);

	friend class MessageDispatcher;

public:
	// req和rsp必须是上层调用者new出来的，释放交给MessageHandlerHolder，上层不需要关注。
	MessageHandlerHolder(ClassType& obj, Method method, google::protobuf::Message* req = 0, google::protobuf::Message* rsp = 0)
		: obj_(obj), method_(method), inMsg_(req), outMsg_(rsp)
	{
	}

	virtual ~MessageHandlerHolder() override
	{
		delete inMsg_;
		delete outMsg_;
	}

	virtual int Exec(const SSProtoHead& h, std::string& outbuf) override
	{
		if (!inMsg_ || inMsg_->ParseFromArray(h.Body, h.ProtoLen - sizeof(SSProtoHead))) {
			if (outMsg_) {
				outMsg_->Clear();
			}

			ErrCodeType err = (obj_.*method_)(h, inMsg_, outMsg_);
			if (err == ErrCodeSucc) {
				outbuf.append(reinterpret_cast<const char*>(&h), sizeof(h));

				uint32_t pkglen = sizeof(h);
				if (outMsg_) {
					if (outMsg_->AppendToString(&outbuf)) {
						pkglen += outMsg_->GetCachedSize();
					} else {
						DEBUG_LOG("Failed to pack outgoing message %s! plid=%u proto=%u target=%u seq=%u",
									outMsg_->GetTypeName().c_str(), h.PlayerID,
									h.ProtoID, h.TargetID, h.SeqNo);
						pack_proto_head(outbuf, h, ErrCodeEncodeFailure);
						return 0;
					}
				}

				outbuf.replace(0, 4, reinterpret_cast<char*>(&pkglen), 4);
				return 0;
			}

			pack_proto_head(outbuf, h, err);
			return 0;

			// 这些代码是如果数据库是MySQL的情况下使用的
//			if (err == dberr_succ) {
//				if (g_sql->commit()) {
//					outbuf.append(reinterpret_cast<char*>(&h), sizeof(h));
//					if (outMsg_) {
//						if (!outMsg_->AppendToString(&outbuf)) {
//							DEBUG_LOG("Failed to pack outgoing message %s! plid=%u proto=%u target=%u seq=%u",
//										outMsg_->GetTypeName().c_str(), h.PlayerID,
//										h.ProtoID, h.TargetID, h.SeqNo);
//							pack_proto_head(outbuf, h, dberr_encode_failure);
//							return 0;
//						}
//					}
//
//					// TODO cached size
//					uint32_t pkglen = (sizeof(SSProtoHead) + out.ByteSize());
//					outbuf.append(reinterpret_cast<char*>(&pkglen), sizeof(pkglen));
//					outbuf.append(reinterpret_cast<const char*>(&(hdr->SeqNo)), sizeof(SSProtoHead) - sizeof(pkglen));
//					if (out.AppendToString(&outbuf)) {
//						return 0;
//					}
//					// TODO
//		//			DEBUG_LOG("Failed to pack outgoing message %s! plid=%u proto=%u target=%u seq=%u",
//		//								hdr->PlayerID, hdr->ProtoID, hdr->TargetID, hdr->SeqNo);
//					pack_proto_head(outbuf, hdr, dberr_encode_failure);
//					return 0;
//				}
//
//				// 如果commit失败，几乎肯定是数据库连接断开，此时再做rollback也没有任何意义
//				err = dberr_dbsys;
//			} else {
//				// rollback即使失败，也没有合适的处理方法，故不判断rollback是否成功
//				g_sql->rollback();
//			}
//
//			pack_proto_head(outbuf, hdr, err);
//			return 0;
		}

		DEBUG_LOG("Failed to parse %s! plid=%u proto=%u target=%u totalLen=%u bodyLen=%lu",
					inMsg_->GetTypeName().c_str(), h.PlayerID, h.ProtoID,
					h.TargetID, h.ProtoLen, h.ProtoLen - sizeof(SSProtoHead));
		pack_proto_head(outbuf, h, ErrCodeInvalidPacket);
		return 0;
	}

	virtual const std::string GetReqName() const override
	{
		if (inMsg_) {
			return inMsg_->GetTypeName();
		}
		return "NIL";
	}

	virtual const std::string GetRspName() const override
	{
		if (outMsg_) {
			return outMsg_->GetTypeName();
		}
		return "NIL";
	}

private:
	ClassType&					obj_;
	Method						method_;
	google::protobuf::Message*	inMsg_;
	google::protobuf::Message*	outMsg_;
};

class MessageDispatcher {
private:
	enum {
		kMaxHandlerNum	= 0xFFFF,
	};

public:
	MessageDispatcher()
	{
		for (int i = 0; i != kMaxHandlerNum; ++i) {
			handlers_[i] = nullptr;
		}
	}

	~MessageDispatcher()
	{
		for (int i = 0; i != kMaxHandlerNum; ++i) {
			delete handlers_[i];
		}
	}

	// req和rsp必须是上层调用者new出来的，释放交给MessageDispatcher，上层不需要关注。
	template<typename ClassType>
	void RegisterHandler(const uint16_t msgID, ClassType& obj, typename MessageHandlerHolder<ClassType>::Method method,
							google::protobuf::Message* req = 0, google::protobuf::Message* rsp = 0)
	{
		if (handlers_[msgID]) {
			throw std::runtime_error("Duplicate message ID " + std::to_string(msgID));
		}
		handlers_[msgID] = new MessageHandlerHolder<ClassType>(obj, method, req, rsp);
	}

	int Dispatch(const char* pkg, std::string& outbuf)
	{
		const SSProtoHead* hdr = reinterpret_cast<const SSProtoHead*>(pkg);
		auto hdlr = handlers_[hdr->ProtoID];
		//DEBUG_LOG("pid %u proto %u len %u", hdr->PlayerID, hdr->ProtoID, hdr->ProtoLen);
		if (hdlr) {
			struct timeval startTv;
			gettimeofday(&startTv, NULL);
			int64_t startms = startTv.tv_sec * 1000 + startTv.tv_usec / 1000;
			int ret = hdlr->Exec(*hdr, outbuf);
			struct timeval endTv;
			gettimeofday(&endTv, NULL);
			int64_t endms = endTv.tv_sec * 1000 + endTv.tv_usec / 1000;
			int64_t interval = endms - startms;
			if (interval > 10) {
				DEBUG_LOG("deal player %u proto %d cost %lu ms", hdr->PlayerID, hdr->ProtoID, interval);
			}
			return ret;
		}

		DEBUG_LOG("Unsupported proto! plid=%u proto=%u target=%u seq=%u",
					hdr->PlayerID, hdr->ProtoID, hdr->TargetID, hdr->SeqNo);
		pack_proto_head(outbuf, *hdr, ErrCodeInvalidProtoID);
		return 0;
	}


	int GetProtoLen(const char* buffer, int length, const skinfo_t* sk)
	{
		if (length > 3) { // 至少需要4个字节才能解包出整包长度
			uint32_t len = *reinterpret_cast<const uint32_t*>(buffer);
			if ((len >= sizeof(SSProtoHead)) && (len <= 1048576)) {
				return len;
			}

			DEBUG_LOG("Invalid package length [%u (limit 1048576)] from [%X]", len, sk->remote_ip);
			return -1;
		}

		return 0;
	}

	uint32_t GetHashCode(const char* buf, uint32_t len)
	{
		assert(len >= sizeof(SSProtoHead));

		const SSProtoHead* h = reinterpret_cast<const SSProtoHead*>(buf);
		if (h->TargetID == 0) {
			return (uint32_t)-1;
		}
		else {
			//return h->TargetID % 10000; 
			return h->TargetID;
		}
	}

	void PrintSupportedProtos();

private:
	AbsMessageHandlerHolder* handlers_[kMaxHandlerNum];
};

#define gMsgDispatcher Singleton<MessageDispatcher>::Instance()

#endif // DIGIMON_DISPATCHER_H_
