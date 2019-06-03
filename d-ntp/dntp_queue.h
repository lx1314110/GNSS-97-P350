#ifndef	__DNTP_QUEUE__
#define	__DNTP_QUEUE__






/*
 * Singly-linked Tail queue declarations.
 */
#define	STAILQ_HEAD(name, type)\
struct name {\
	struct type *stqh_first;/* first element */\
	struct type **stqh_last;/*指向最后一个元素的stqe_next*/\
}






#define	STAILQ_HEAD_INITIALIZER(head)\
	{ NULL, &(head).stqh_first }






#define	STAILQ_ENTRY(type)\
struct {\
	struct type *stqe_next;/* next element */\
}







/*
 * Singly-linked Tail queue functions.
 */
#define	STAILQ_EMPTY(head)		((head)->stqh_first == NULL)






#define	STAILQ_FIRST(head)		((head)->stqh_first)







#define	STAILQ_FOREACH(var, head, field)\
	for((var) = STAILQ_FIRST((head)); (var); (var) = STAILQ_NEXT((var), field))







#define	STAILQ_INIT(head) do {\
	STAILQ_FIRST((head)) = NULL;\
	(head)->stqh_last = &STAILQ_FIRST((head));\
} while(0)








#define	STAILQ_INSERT_AFTER(head, tqelm, elm, field) do {\
	if ((STAILQ_NEXT((elm), field) = STAILQ_NEXT((tqelm), field)) == NULL)\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);\
	STAILQ_NEXT((tqelm), field) = (elm);\
} while(0)








#define	STAILQ_INSERT_HEAD(head, elm, field) do {\
	if ((STAILQ_NEXT((elm), field) = STAILQ_FIRST((head))) == NULL)\
		(head)->stqh_last = &STAILQ_NEXT((elm), field);\
	STAILQ_FIRST((head)) = (elm);\
} while(0)








#define	STAILQ_INSERT_TAIL(head, elm, field) do {\
	STAILQ_NEXT((elm), field) = NULL;\
	STAILQ_LAST((head)) = (elm);\
	(head)->stqh_last = &STAILQ_NEXT((elm), field);\
} while(0)







#define	STAILQ_LAST(head)	(*(head)->stqh_last)






#define	STAILQ_NEXT(elm, field)	((elm)->field.stqe_next)







#define	STAILQ_REMOVE(head, elm, type, field) do {\
	if (STAILQ_FIRST((head)) == (elm)) {\
		STAILQ_REMOVE_HEAD(head, field);\
	}\
	else {\
		struct type *curelm = STAILQ_FIRST((head));\
		while (STAILQ_NEXT(curelm, field) != (elm))\
			curelm = STAILQ_NEXT(curelm, field);\
		if ((STAILQ_NEXT(curelm, field) = STAILQ_NEXT(STAILQ_NEXT(curelm, field), field)) == NULL)\
			(head)->stqh_last = &STAILQ_NEXT((curelm), field);\
	}\
} while(0)








#define	STAILQ_REMOVE_HEAD(head, field) do {\
	if ((STAILQ_FIRST((head)) = STAILQ_NEXT(STAILQ_FIRST((head)), field)) == NULL)\
		(head)->stqh_last = &STAILQ_FIRST((head));\
} while(0)








#define	STAILQ_REMOVE_HEAD_UNTIL(head, elm, field) do {\
	if ((STAILQ_FIRST((head)) = STAILQ_NEXT((elm), field)) == NULL)\
		(head)->stqh_last = &STAILQ_FIRST((head));\
} while(0)









#endif//__DNTP_QUEUE__


