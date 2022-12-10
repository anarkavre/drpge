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

#ifndef __CNODE_H__
#define __CNODE_H__

template <class T>
class CNode
{
	template <class T>
	friend class CList;

public:
	CNode(T *data = nullptr, bool nodeOwnsData = true) : m_data(data), m_nodeOwnsData(nodeOwnsData), m_refCount(nullptr), m_visitCount(nullptr), m_prev(nullptr), m_next(nullptr) {}

	T *GetData() const { return m_data; }
	void SetData(T *data) { m_data = data; }

	unsigned int GetRefCount() const { return (m_refCount != nullptr ? *m_refCount : 1); }
	unsigned int VisitNode() { if (m_visitCount != nullptr) { if (*m_visitCount == 0) *m_visitCount = *m_refCount; return --(*m_visitCount); } else return 0; }

	CNode<T> *Prev() { return m_prev; };
	CNode<T> *Next() { return m_next; };

private:
	T *m_data;
	bool m_nodeOwnsData;
	unsigned int *m_refCount;
	unsigned int *m_visitCount;
	CNode<T> *m_prev;
	CNode<T> *m_next;
};

#endif