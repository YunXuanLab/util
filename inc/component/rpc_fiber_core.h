//
// Created by hujianzhe
//

#ifndef UTIL_C_COMPONENT_RPC_FIBER_CORE_H
#define	UTIL_C_COMPONENT_RPC_FIBER_CORE_H

#include "../datastruct/rbtree.h"
#include "../sysapi/process.h"

typedef struct RpcItem_t {
	RBTreeNode_t m_treenode;
	int id;
	long long timestamp_msec;
	long long timeout_msec;
	void* req_ctx;
	void* ret_msg;
} RpcItem_t;

typedef struct RpcFiberCore_t {
	Fiber_t* fiber;
	Fiber_t* sche_fiber;
	RBTree_t reg_tree;
	void* new_msg;
	void* disconnect_cmd;
	void(*msg_handler)(struct RpcFiberCore_t*, void*);
} RpcFiberCore_t;

typedef struct RpcAsyncCore_t {
	RBTree_t reg_tree;
} RpcAsyncCore_t;

#ifdef __cplusplus
extern "C" {
#endif

RpcAsyncCore_t* rpcAsyncCoreInit(RpcAsyncCore_t* rpc);
void rpcAsyncCoreDestroy(RpcAsyncCore_t* rpc);
RpcItem_t* rpcAsyncCoreRegItem(RpcAsyncCore_t* rpc, int rpcid, long long timeout_msec, void* req_ctx);
RpcItem_t* rpcAsyncCoreExistItem(RpcAsyncCore_t* rpc, int rpcid);
void rpcAsyncCoreFreeItem(RpcAsyncCore_t* rpc, RpcItem_t* item);

RpcFiberCore_t* rpcFiberCoreInit(RpcFiberCore_t* rpc, Fiber_t* sche_fiber, size_t stack_size);
void rpcFiberCoreDestroy(RpcFiberCore_t* rpc);
RpcItem_t* rpcFiberCoreExistItem(RpcFiberCore_t* rpc, int rpcid);
void rpcFiberCoreFreeItem(RpcFiberCore_t* rpc, RpcItem_t* item);
RpcItem_t* rpcFiberCoreReturnWait(RpcFiberCore_t* rpc, int rpcid, long long timeout_msec);
int rpcFiberCoreReturnSwitch(RpcFiberCore_t* rpc, int rpcid, void* ret_msg);
void rpcFiberCoreMessageHandleSwitch(RpcFiberCore_t* rpc, void* new_msg);
void rpcFiberCoreDisconnectHandleSwitch(RpcFiberCore_t* rpc, void* disconnect_cmd);

#ifdef __cplusplus
}
#endif

#endif // !UTIL_C_COMPONENT_RPC_FIBER_CORE_H
