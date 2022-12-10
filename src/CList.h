// drpge
// Copyright(C) 2020-2022 John D. Corrado
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#ifndef __CLIST_H__
#define __CLIST_H__

#include "CNode.h"

template <class T>
class CList
{
public:
	CList();
	~CList();

	CNode<T> *Insert(T *data, bool nodeOwnsData = true, CNode<T> *nodeToInsertAfter = nullptr);
	CNode<T> *Insert(CNode<T> *refNode, CNode<T> *nodeToInsertAfter = nullptr);
	void Delete(CNode<T> *nodeToDelete);
	void Delete(CNode<T> *start, CNode<T> *end);
	void Delete(T *data);
	void Reverse(CNode<T> *start = nullptr, CNode<T> *end = nullptr);

	CNode<T> *Head() const { return m_head.m_next; };
	CNode<T> *Tail() const { return m_head.m_prev; };

	bool IsEmpty() const { return m_head.m_prev->m_data == nullptr; }
	unsigned int Size() const { return m_size; }
	unsigned int UniqueSize() const { return m_uniqueSize; }

private:
	CNode<T> m_head;
	unsigned int m_size;
	unsigned int m_uniqueSize;
};

template <class T>
CList<T>::CList() : m_size(0), m_uniqueSize(0)
{
	m_head.m_prev = m_head.m_next = &m_head;
}

template <class T>
CList<T>::~CList()
{
	CNode<T> *currentNode = m_head.m_next;

	while (currentNode->m_data != nullptr)
	{
		CNode<T> *nodeToDelete = currentNode;
		currentNode = currentNode->m_next;

		if (nodeToDelete->m_refCount == nullptr)
		{
			if (nodeToDelete->m_nodeOwnsData)
				delete nodeToDelete->m_data;
		}
		else if (--nodeToDelete->m_refCount == 0)
		{
			delete nodeToDelete->m_visitCount;
			delete nodeToDelete->m_refCount;

			if (nodeToDelete->m_nodeOwnsData)
				delete nodeToDelete->m_data;
		}

		delete nodeToDelete;
	}
}

template <class T>
CNode<T> *CList<T>::Insert(T *data, bool nodeOwnsData, CNode<T> *nodeToInsertAfter)
{
	if (data == nullptr)
		return nullptr;

	nodeToInsertAfter = (nodeToInsertAfter != nullptr ? nodeToInsertAfter : m_head.m_prev);
	CNode<T> *newNode = new CNode<T>;
	newNode->m_data = data;
	newNode->m_nodeOwnsData = nodeOwnsData;
	newNode->m_prev = nodeToInsertAfter;
	newNode->m_next = nodeToInsertAfter->m_next;
	nodeToInsertAfter->m_next->m_prev = newNode;
	nodeToInsertAfter->m_next = newNode;

	m_size++;
	m_uniqueSize++;

	return newNode;
}

template <class T>
CNode<T> *CList<T>::Insert(CNode<T> *refNode, CNode<T> *nodeToInsertAfter)
{
	nodeToInsertAfter = (nodeToInsertAfter != nullptr ? nodeToInsertAfter : m_head.m_prev);
	CNode<T> *newNode = new CNode<T>;
	newNode->m_data = refNode->m_data;
	newNode->m_nodeOwnsData = refNode->m_nodeOwnsData;
	newNode->m_prev = nodeToInsertAfter;
	newNode->m_next = nodeToInsertAfter->m_next;
	nodeToInsertAfter->m_next->m_prev = newNode;
	nodeToInsertAfter->m_next = newNode;

	if (refNode->m_refCount == nullptr)
	{
		refNode->m_refCount = new unsigned int;
		refNode->m_visitCount = new unsigned int;
		*refNode->m_refCount = *refNode->m_visitCount = 2;
	}
	else
		(*refNode->m_refCount)++;

	newNode->m_refCount = refNode->m_refCount;
	newNode->m_visitCount = refNode->m_visitCount;

	m_size++;

	return newNode;
}

template <class T>
void CList<T>::Delete(CNode<T> *nodeToDelete)
{
	if (nodeToDelete == nullptr)
		return;

	nodeToDelete->m_prev->m_next = nodeToDelete->m_next;
	nodeToDelete->m_next->m_prev = nodeToDelete->m_prev;

	if (nodeToDelete->m_refCount == nullptr)
	{
		if (nodeToDelete->m_nodeOwnsData)
			delete nodeToDelete->m_data;

		m_uniqueSize--;
	}
	else if (--(*nodeToDelete->m_refCount) == 0)
	{
		delete nodeToDelete->m_visitCount;
		delete nodeToDelete->m_refCount;

		if (nodeToDelete->m_nodeOwnsData)
			delete nodeToDelete->m_data;

		m_uniqueSize--;
	}

	delete nodeToDelete;

	m_size--;
}

template <class T>
void CList<T>::Delete(CNode<T> *startNode = nullptr, CNode<T> *endNode = nullptr)
{
	CNode<T> *currentNode = (startNode != nullptr ? startNode : m_head.m_next);
	endNode = (endNode != nullptr ? endNode : &m_head);
	currentNode->m_prev->m_next = endNode;
	endNode->m_prev = currentNode->m_prev;

	while (currentNode != endNode)
	{
		CNode<T> *nodeToDelete = currentNode;
		currentNode = currentNode->m_next;

		if (nodeToDelete->m_refCount == nullptr)
		{
			if (nodeToDelete->m_nodeOwnsData)
				delete nodeToDelete->m_data;

			m_uniqueSize--;
		}
		else if (--(*nodeToDelete->m_refCount) == 0)
		{
			delete nodeToDelete->m_visitCount;
			delete nodeToDelete->m_refCount;

			if (nodeToDelete->m_nodeOwnsData)
				delete nodeToDelete->m_data;

			m_uniqueSize--;
		}

		delete nodeToDelete;

		m_size--;
	}
}

template <class T>
void CList<T>::Delete(T *data)
{
	CNode<T> *currentNode = m_head.m_next;

	while (currentNode->m_data != nullptr)
	{
		if (currentNode->GetData() == data)
		{
			CNode<T> *nodeToDelete = currentNode;
			currentNode = currentNode->m_next;

			unsigned int *pVisitCount = nodeToDelete->m_visitCount;
			unsigned int *pRefCount = nodeToDelete->m_refCount;
			unsigned int *pData = nodeToDelete->data;
			bool nodeOwnsData = nodeToDelete->m_nodeOwnsData;

			delete nodeToDelete;

			m_size--;

			if (pRefCount == nullptr)
			{
				if (nodeOwnsData)
					delete pData;

				m_uniqueSize--;

				return;
			}
			else if (--(*pRefCount) == 0)
			{
				delete pVisitCount;
				delete pRefCount;

				if (nodeOwnsData)
					delete pData;

				m_uniqueSize--;

				return;
			}
		}
	}
}

template <class T>
void CList<T>::Reverse(CNode<T> *startNode = nullptr, CNode<T> *endNode = nullptr)
{
	CNode<T> *currentNode = (startNode != nullptr ? startNode : m_head.m_next);
	startNode = currentNode->m_prev;
	endNode = (endNode != nullptr ? endNode : &m_head);

	while (currentNode != endNode)
	{
		CNode<T> *tempNode = currentNode->m_next;
		currentNode->m_next = currentNode->m_prev;
		currentNode->m_prev = tempNode;

		if (currentNode->m_prev == endNode)
		{
			startNode->m_next->m_next = endNode;
			endNode->m_prev = startNode->m_next;
			startNode->m_next = currentNode;
		}

		currentNode = currentNode->m_prev;
	}

	startNode->m_next->m_prev = startNode;
}

#endif