/**
*  @defgroup api_msg
*  @ingroup common_libs
*
*  消息处理函数及关键宏
*
*  @{
*/
#ifndef COMMON_LIBS_MSG_H
#define COMMON_LIBS_MSG_H

#define MSG_STATUS_NEW		(1 << 1)
#define MSG_STATUS_USED_BIT	(1 << 3)

#define MSG_FLAGS_ADDRESS	(1 << 4)
#define MSG_FLAGS_SYNC		(1 << 5)

#define __data_size__(DATA_COUNT) ((DATA_COUNT)  * sizeof(MSG_DATA_TYPE))
#define __msg_count__(DATA_COUNT) ((__data_size__(DATA_COUNT) + sizeof(struct message) - 1)/*max possible size*/ / sizeof(struct message))
#define MSG_MAKE(DATA_COUNT, FLAGS, NAME) \
	struct message __msg__[__msg_count__(DATA_COUNT)/* Data part */ + 1/*The head of msg*/];	\
	struct message *pmsg = &__msg__[0];	\
	unsigned long *pdata = (unsigned long*) &__msg__[1];	\
	pmsg->count = __msg_count__(DATA_COUNT);	\
	pmsg->flags = FLAGS;	\
	pmsg->what = (unsigned long)NAME;

/**
	@brief Data ops on received msg
*/
typedef unsigned long * MSG_PDATA_TYPE;
typedef unsigned long MSG_DATA_TYPE;
#define MSG_DATA_START(DESC, ORG, CUR) CUR = (MSG_PDATA_TYPE)ORG; \
	MSG_GET_NEXT_SLOT(DESC, CUR/*Head is a msg node*/);

#define MSG_DATA_READ_NEXT(DESC, CUR, DATA, TYPE) do { \
	DATA = *(TYPE*)CUR; \
	MSG_GET_NEXT_DATA(DESC, CUR); \
} while (0)
	
#define MSG_DATA_WRITE_NEXT(DESC, CUR, DATA) do { \
	*CUR = (unsigned long)DATA; \
	MSG_GET_NEXT_DATA(DESC, CUR); \
} while (0)
	
/**
	@brief Get next slot
*/
#define MSG_GET_NEXT_SLOT(DESC, CUR) do { \
	CUR = (void*)CUR + sizeof(struct message);/* each node is a slot */ \
	if ((unsigned long)((DESC)->slots) + ((DESC)->slot_buffer_size)/* Buffer limit */ <= (unsigned long)CUR) \
		CUR = (DESC)->slots; \
} while (0)

#define MSG_GET_NEXT_DATA(DESC, CUR) do { \
	CUR = (void*)CUR + sizeof(MSG_DATA_TYPE);/* each node is a DATA */ \
	if ((unsigned long)((DESC)->slots) + ((DESC)->slot_buffer_size)/* Buffer limit */ <= (unsigned long)CUR) \
		CUR = (DESC)->slots; \
} while (0)
struct message
{
	unsigned short count;
	volatile unsigned short flags;
	unsigned long what;	
};
struct message_instance;

typedef bool (*message_filter)(struct message *what);
typedef void (*message_sleep)(struct message_instance *msg_instance);
typedef void (*message_response_sync)(struct message *what);

struct message_instance
{
	message_filter filter;
	message_sleep sleep;
	message_response_sync response_sync;
	void * slots;
	void * current_slot;
	int slot_buffer_size;
};

/**
	@brief goto into message loop

	The function will not exist. When fetch a message it tries to handle it.
*/
struct message * message_loop(struct message_instance * instance);

/**
	@brief 将所有消息的状态初始化

	一般用于初始化刚刚创立的消息缓冲区
*/
void message_reset_all(struct message_instance * instance);

#endif
/** @} */
