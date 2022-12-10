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

#ifndef __CMAP_H__
#define __CMAP_H__

#include "SDL.h"

#include "CGrid.h"
#include "CList.h"

struct Vertex
{
	float x;
	float y;
};

struct Sector;

struct Line
{
	CNode<Vertex> *vertex1;
	CNode<Vertex> *vertex2;
	Sector *sectors[2];
};


struct Sector
{
	float minX;
	float minY;
	float maxX;
	float maxY;
	CNode<Vertex> *firstVertex;
	CNode<Vertex> *lastVertex;
	CNode<Line> *firstLine;
	CNode<Line> *lastLine;
	unsigned int vertexCount;
	unsigned int lineCount;
};

struct Thing
{
	float x;
	float y;
};

class CMap
{
public:
	CMap() {}

	void Read(const char *filename);
	void Write(const char *filename);
	void Render(SDL_Renderer *renderer, CGrid &grid);

	CList<Vertex> *GetVertices() { return &m_vertices; }
	CList<Line> *GetLines() { return &m_lines; }
	CList<Sector> *GetSectors() { return &m_sectors; }
	CList<Thing> *GetThings() { return &m_things; }

private:
	CList<Vertex> m_vertices;
	CList<Line> m_lines;
	CList<Sector> m_sectors;
	CList<Thing> m_things;
	unsigned char m_blockMap[32][32];
};

#endif