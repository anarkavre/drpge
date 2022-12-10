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

#include <vector>

#include "CMap.h"
#include "doomrpg_data.h"

using namespace std;

void CMap::Read(const char *filename)
{
	bspmapex_t *map = LoadBspMapEx(filename);
	CList<Vertex> vertices;
	CList<Line> lines;

	for (uint32_t i = 0; i < map->lineCount; i++)
	{
		linesegmentex_t *line = &map->lines[i];

		CNode<Vertex> *vertex1 = m_vertices.Insert(new Vertex({ float(line->start.x * 8), float(line->start.y * 8) }));
		CNode<Vertex> *vertex2 = m_vertices.Insert(new Vertex({ float(line->end.x * 8), float(line->end.y * 8) }));
		m_lines.Insert(new Line({ vertex1, vertex2 }));
	}

	/*CNode<Line> *currentLine = lines.Head();

	Sector sector;
	sector.vertexCount = 0;
	sector.lineCount = 0;
	sector.minX = sector.minY = FLT_MAX;
	sector.maxX = sector.maxY = -FLT_MAX;

	while (!lines.IsEmpty())
	{
		CNode<Vertex> *vertex1 = m_vertices.Insert(new Vertex(*currentLine->GetData()->vertex1->GetData()));
		CNode<Vertex> *vertex2 = m_vertices.Insert(new Vertex(*currentLine->GetData()->vertex2->GetData()));
		CNode<Line> *line = m_lines.Insert(new Line({ vertex1, vertex2 }));
		sector.firstLine = line;
		sector.vertexCount += 2;
		sector.lineCount++;
		vertices.Delete(currentLine->GetData()->vertex1);
		vertices.Delete(currentLine->GetData()->vertex2);
		lines.Delete(currentLine);

		for (currentLine = lines.Head(); currentLine != lines.Tail(); currentLine = currentLine->Next())
		{
			if (currentLine->GetData()->vertex1->GetData()->x == vertex2->GetData()->x && currentLine->GetData()->vertex1->GetData()->y == vertex2->GetData()->y)
			{
				vertex1 = m_vertices.Insert(vertex2);
				vertex2 = m_vertices.Insert(new Vertex(*currentLine->GetData()->vertex2->GetData()));
				line = m_lines.Insert(new Line({ vertex1, vertex2 }));
				sector.vertexCount += 2;
				sector.lineCount++;
				vertices.Delete(currentLine->GetData()->vertex1);
				vertices.Delete(currentLine->GetData()->vertex2);
				CNode<Line> *previousLine = currentLine->Prev();
				lines.Delete(currentLine);
				currentLine = previousLine;

				if (vertex2->GetData()->x == sector.firstLine->GetData()->vertex1->GetData()->x && vertex2->GetData()->y == sector.firstLine->GetData()->vertex1->GetData()->y)
				{
					sector.lastLine = line;
					m_sectors.Insert(new Sector(sector));
					sector = Sector();
					sector.vertexCount = 0;
					sector.lineCount = 0;
					sector.minX = sector.minY = FLT_MAX;
					sector.maxX = sector.maxY = -FLT_MAX;
					break;
				}
			}
		}
	}*/

	for (uint32_t i = 0; i < map->thingCount; i++)
	{
		thing_t *thing = &map->things[i];

		if ((thing->flags & 0x802) == 0x802)
		{
			CNode<Vertex> *vertex1 = nullptr;
			CNode<Vertex> *vertex2 = nullptr;

			if (thing->flags & 0x8)
			{
				vertex1 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8 + 32), float(thing->position.y * 8) }));
				vertex2 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8 - 32), float(thing->position.y * 8) }));
			}
			else if (thing->flags & 0x10)
			{
				vertex1 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8 - 32), float(thing->position.y * 8) }));
				vertex2 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8 + 32), float(thing->position.y * 8) }));
			}
			else if (thing->flags & 0x20)
			{
				vertex1 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8), float(thing->position.y * 8 + 32) }));
				vertex2 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8), float(thing->position.y * 8 - 32) }));
			}
			else if (thing->flags & 0x40)
			{
				vertex1 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8), float(thing->position.y * 8 - 32) }));
				vertex2 = m_vertices.Insert(new Vertex({ float(thing->position.x * 8), float(thing->position.y * 8 + 32) }));
			}

			m_lines.Insert(new Line({ vertex1, vertex2 }));
		}
		else
			m_things.Insert(new Thing({ float(thing->position.x * 8), float(thing->position.y * 8) }));
	}

	for (unsigned int y = 0; y < 32; y++)
	{
		for (unsigned int x = 0; x < 32; x++)
		{
			m_blockMap[y][x] = (map->blockMap[y * 8 + (x / 4)] >> ((x % 4) * 2)) & 0x3;
		}
	}

	FreeBspMapEx(map);
}

void CMap::Write(const char *filename)
{
}

void CMap::Render(SDL_Renderer *renderer, CGrid &grid)
{
	/*for (unsigned int y = 0; y < 32; y++)
	{
		for (unsigned int x = 0; x < 32; x++)
		{
			unsigned int color = 255 * m_blockMap[y][x] / 3;

			SDL_SetRenderDrawColor(renderer, color, color, color, 255);

			SDL_Rect rect;
			rect.x = int(grid.TranslateXToViewSpace(x * 64.0f));
			rect.y = int(grid.TranslateYToViewSpace(y * 64.0f));
			rect.w = int(64 * grid.GetScale());
			rect.h = int(64 * grid.GetScale());

			SDL_RenderFillRect(renderer, &rect);
		}
	}*/

	if (!m_lines.IsEmpty())
	{
		for (CNode<Line> *currentLine = m_lines.Head(); currentLine->GetData() != nullptr; currentLine = currentLine->Next())
		{
			if (currentLine->VisitNode() == 0)
			{
				const Vertex *vertex1 = currentLine->GetData()->vertex1->GetData();
				const Vertex *vertex2 = currentLine->GetData()->vertex2->GetData();
				int x1 = int(grid.TranslateXToViewSpace(vertex1->x));
				int y1 = int(grid.TranslateYToViewSpace(vertex1->y));
				int x2 = int(grid.TranslateXToViewSpace(vertex2->x));
				int y2 = int(grid.TranslateYToViewSpace(vertex2->y));

				if (currentLine->GetRefCount() == 1)
					SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				else
					SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);

				SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
			}
		}

		vector<SDL_Rect> rects;

		for (CNode<Vertex> *currentVertex = m_vertices.Head(); currentVertex->GetData() != nullptr; currentVertex = currentVertex->Next())
		{
			if (currentVertex->VisitNode() == 0)
			{
				const Vertex *vertex = currentVertex->GetData();
				int x = int(grid.TranslateXToViewSpace(vertex->x));
				int y = int(grid.TranslateYToViewSpace(vertex->y));
				rects.push_back({ x - 2, y - 2, 5, 5 });
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
		SDL_RenderFillRects(renderer, rects.data(), rects.size());
	}

	if (!m_things.IsEmpty())
	{
		vector<SDL_Rect> rects;

		for (CNode<Thing> *currentThing = m_things.Head(); currentThing->GetData() != nullptr; currentThing = currentThing->Next())
		{
			if (currentThing->VisitNode() == 0)
			{
				const Thing *thing = currentThing->GetData();
				int x = int(grid.TranslateXToViewSpace(thing->x));
				int y = int(grid.TranslateYToViewSpace(thing->y));
				rects.push_back({ x - grid.GetScaledCellSize() / 2, y - grid.GetScaledCellSize() / 2, grid.GetScaledCellSize() + 1, grid.GetScaledCellSize() + 1 });
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRects(renderer, rects.data(), rects.size());
	}
}