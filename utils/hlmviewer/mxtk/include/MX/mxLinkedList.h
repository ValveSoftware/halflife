//
//                 mxToolKit (c) 1999 by Mete Ciragan
//
// file:           mxLinkedList.h
// implementation: all
// last modified:  Mar 19 1999, Mete Ciragan
// copyright:      The programs and associated files contained in this
//                 distribution were developed by Mete Ciragan. The programs
//                 are not in the public domain, but they are freely
//                 distributable without licensing fees. These programs are
//                 provided without guarantee or warrantee expressed or
//                 implied.
//
#ifndef INCLUDED_MXLINKEDLIST
#define INCLUDED_MXLINKEDLIST



typedef struct mxListNode_s
{
	void *d_data;
	struct mxListNode_s *d_next;
	struct mxListNode_s *d_prev;
} mxListNode;



class mxLinkedList
{
	mxListNode *d_head;
	mxListNode *d_tail;
	int d_nodeCount;

	// NOT IMPLEMENTED
	mxLinkedList (const mxLinkedList&);
	mxLinkedList& operator= (const mxLinkedList&);

public:
	//CREATORS
	mxLinkedList ()
	{
		d_head = new mxListNode;
		d_tail = new mxListNode;
		d_head->d_data = 0;
		d_head->d_next = d_tail;
		d_head->d_prev = 0;
		d_tail->d_data = 0;
		d_tail->d_next = 0;
		d_tail->d_prev = d_head;
		d_nodeCount = 0;
	}

	~mxLinkedList ()
	{
		removeAll ();
		delete d_tail;
		delete d_head;
	}

	// MANIPULATORS
	void add (void *data)
	{
		mxListNode *node = new mxListNode;
		node->d_data = data;
		d_tail->d_prev->d_next = node;
		node->d_prev = d_tail->d_prev;
		node->d_next = d_tail;
		d_tail->d_prev = node;
		++d_nodeCount;
	}

	void remove (void *data)
	{
		mxListNode *node = d_head->d_next;
		while (node != d_tail)
		{
			mxListNode *next = node->d_next;
			if (node->d_data == data)
			{
				node->d_prev->d_next = node->d_next;
				node->d_next->d_prev = node->d_prev;
				delete node;
			}

			node = next;
		}
		--d_nodeCount;
	}

	void removeAll ()
	{
		mxListNode *node = d_head->d_next;

		while (node != d_tail)
		{
			mxListNode *next = node->d_next;
			delete node;
			node = next;
		}

		d_head->d_next = d_tail;
		d_tail->d_prev = d_head;
		d_nodeCount = 0;
	}

	void setData (mxListNode *node, void *data)
	{
		if (node)
			node->d_data = data;
	}

	// ACCESSORS
	void *getData (mxListNode *node) const
	{
		if (node)
			return node->d_data;

		return 0;
	}

	mxListNode *getFirst () const
	{
		if (d_head->d_next != d_tail)
			return d_head->d_next;

		return 0;
	}

	mxListNode *getNext (mxListNode *node) const
	{
		if (node)
		{
			if (node->d_next != d_tail)
				return node->d_next;

			return 0;
		}

		return 0;
	}

	mxListNode *getLast () const
	{
		if (d_tail->d_prev != d_head)
			return d_tail->d_prev;

		return 0;
	}

	mxListNode *getPrev (mxListNode *node) const
	{
		if (node)
		{
			if (node->d_prev != d_head)
				return node->d_prev;

			return 0;
		}

		return 0;
	}

	mxListNode *at (int pos) const
	{
		mxListNode *node = d_head->d_next;
		while (pos > 0 && node != d_tail)
		{
			pos--;
			node = node->d_next;
		}

		if (node != d_tail)
			return node;

		return 0;
	}

	bool isEmpty () const
	{
		return (d_head->d_next == d_tail);
	}

	int getNodeCount () const
	{
		return d_nodeCount;
	}
};



#endif // INCLUDED_MXLINKEDLIST