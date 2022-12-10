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

#include "SDL.h"
#include <string>
#include <vector>

#include "CGrid.h"
#include "CList.h"
#include "CMap.h"

using namespace std;

enum Mode
{
	MODE_DRAW,
	MODE_MOVE,
	MODE_VERTEX,
	MODE_NONE
};

enum Selection
{
	SELECTION_VERTEX,
	SELECTION_LINE,
	SELECTION_SECTOR,
	SELECTION_NONE
};

bool SectorIsClockwise(const Sector &sector);
bool AABBContainsPoint(float x, float y, float minX, float minY, float maxX, float maxY);
Selection FindSelection(CList<Sector> *sectors, float x, float y, CNode<Sector> **selectedSector, CNode<Line> **selectedLine, CNode<Vertex> **selectedVertex);
CNode<Line> *FindLine(const CList<Line> *lines, const Vertex *vertex1, const Vertex *vertex2);
void ProjectPointOnSegment(const Vertex &vertex1, const Vertex &vertex2, float x, float y, Vertex &vertexOut);
void CalculateSectorAABB(Sector &sector);
void InitializeSector(Sector &sector);
void CancelSector(CMap &map, Sector &sector);
void DeleteSector(CMap &map, CNode<Sector> &sector);
CNode<Vertex> *InsertVertex(CMap &map, Vertex &vertex);
CNode<Line> *InsertLine(CMap &map, Line &line);
CNode<Sector> *InsertSector(CMap &map, Sector &sector);
void CloseSector(CMap &map, Sector &sector, Line &line);
void MoveVertex(Vertex &vertex, int x, int y, CGrid &grid);
void MoveLine(Line &line, int x, int y, int referenceX, int referenceY, int &initialX, int &initialY, float scaleInverse, CGrid &grid);
void MoveSector(Sector &sector, int x, int y, int referenceX, int referenceY, int &initialX, int &initialY, float scaleInverse, CGrid &grid);
void RecalculateSectorsAABB(CMap &map, CNode<Vertex> &vertex);
void RecalculateSectorsAABB(CMap &map, CNode<Line> &line);
void RecalculateSectorsAABB(CMap &map, CNode<Sector> &sector);

int main(int argc, char *argv[]) {
	char *filename = nullptr;

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			if (!strcmp(argv[i], "-map"))
				filename = argv[i + 1];
		}
	}

	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Doom RPG Edit - Mode: Draw - Zoom: 25%", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 515, 515, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	CGrid grid(515, 515, 8, 0, 0, 0.25f);

	grid.SetMinX(0);
	grid.SetMinY(0);
	grid.SetMaxX(256);
	grid.SetMaxY(256);

	grid.SetXDisplacement(-grid.GetScaledCellSize() * grid.GetMaxX() / 2);
	grid.SetYDisplacement(-grid.GetScaledCellSize() * grid.GetMaxY() / 2);

	CMap map;
	map.Read(filename);

	Mode mode = MODE_DRAW;

	bool drawing = false;
	bool moving = false;
	bool scrolling = false;

	float x, y;

	Sector sector;
	InitializeSector(sector);

	Line line;
	line.sectors[0] = nullptr;
	line.sectors[1] = nullptr;

	CNode<Sector> *selectedSector = nullptr;
	CNode<Line> *selectedLine = nullptr;
	CNode<Vertex> *selectedVertex = nullptr;

	Selection selection = SELECTION_NONE;

	int referenceX, referenceY;
	int initialX, initialY;

	float scale = 0.25f;
	float scaleInverse = 4.0f;

	bool updateTitle = false;

	bool running = true;

	while (running)
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					if (drawing)
					{
						CancelSector(map, sector);
						InitializeSector(sector);
						drawing = false;
					}
					break;
				case SDLK_RETURN:
					if (drawing && sector.vertexCount > 4)
					{
						CloseSector(map, sector, line);
						InitializeSector(sector);
						drawing = false;
					}

					break;
				case SDLK_DELETE:
					if (mode == MODE_MOVE && selection == SELECTION_SECTOR)
					{
						DeleteSector(map, *selectedSector);
						selection = SELECTION_NONE;
					}

					break;
				case SDLK_q:
					running = false;
					break;
				case SDLK_d:
					if (!moving)
					{
						mode = MODE_DRAW;
						updateTitle = true;
					}

					break;
				case SDLK_m:
					if (!drawing)
					{
						mode = MODE_MOVE;
						updateTitle = true;
						int x, y;
						SDL_GetMouseState(&x, &y);
						selection = FindSelection(map.GetSectors(), grid.TranslateXToGridSpace(float(x)), grid.TranslateYToGridSpace(float(y)), &selectedSector, &selectedLine, &selectedVertex);
					}

					break;
				case SDLK_v:
					if (!drawing && !moving)
					{
						mode = MODE_VERTEX;
						updateTitle = true;
					}

					break;
				case SDLK_c:
					grid.CenterOrigin();
					grid.SetXDisplacement(-grid.GetScaledCellSize() * grid.GetMaxX() / 2);
					grid.SetYDisplacement(-grid.GetScaledCellSize() * grid.GetMaxY() / 2);
					break;
				case SDLK_LEFT:
					grid.ScrollX(grid.GetScaledCellSize());
					break;
				case SDLK_RIGHT:
					grid.ScrollX(-grid.GetScaledCellSize());
					break;
				case SDLK_UP:
					grid.ScrollY(grid.GetScaledCellSize());
					break;
				case SDLK_DOWN:
					grid.ScrollY(-grid.GetScaledCellSize());
					break;
				case SDLK_MINUS:
				case SDLK_KP_MINUS:
					if (scale > 0.25f)
						scale -= 0.25f;

					scaleInverse = 1.0f / scale;

					grid.SetScale(scale);

					updateTitle = true;

					break;
				case SDLK_EQUALS:
				case SDLK_KP_PLUS:
					if (scale < 2.0f)
						scale += 0.25f;

					scaleInverse = 1.0f / scale;

					grid.SetScale(scale);

					updateTitle = true;

					break;
				default:
					break;
				}

				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT && !scrolling)
				{
					if (mode == MODE_DRAW)
					{
						Vertex vertex;

						x = float(event.button.x);
						y = float(event.button.y);

						grid.Snap(x, y);

						vertex.x = x;
						vertex.y = y;

						if (sector.vertexCount > 0 && vertex.x == map.GetVertices()->Tail()->GetData()->x && vertex.y == map.GetVertices()->Tail()->GetData()->y)
							break;

						if (sector.vertexCount > 2 && AABBContainsPoint(vertex.x, vertex.y, sector.firstVertex->GetData()->x - 2, sector.firstVertex->GetData()->y - 2, sector.firstVertex->GetData()->x + 2, sector.firstVertex->GetData()->y + 2))
						{
							CloseSector(map, sector, line);
							InitializeSector(sector);
							drawing = false;
						}
						else
						{
							line.vertex2 = InsertVertex(map, vertex);

							if (line.vertex2->GetData()->x < sector.minX)
								sector.minX = line.vertex2->GetData()->x;

							if (line.vertex2->GetData()->y < sector.minY)
								sector.minY = line.vertex2->GetData()->y;

							if (line.vertex2->GetData()->x > sector.maxX)
								sector.maxX = line.vertex2->GetData()->x;

							if (line.vertex2->GetData()->y > sector.maxY)
								sector.maxY = line.vertex2->GetData()->y;

							if ((++sector.vertexCount % 2) == 0)
							{
								CNode<Line> *newLineNode = InsertLine(map, line);

								if (sector.lineCount++ == 0)
									sector.firstLine = sector.lastLine = newLineNode;
								else
									sector.lastLine = newLineNode;

								sector.vertexCount++;
							}

							line.vertex1 = line.vertex2;

							if (sector.vertexCount == 1)
								sector.firstVertex = sector.lastVertex = line.vertex1;
							else
								sector.lastVertex = line.vertex1;

							drawing = true;
						}
					}
					else if (mode == MODE_MOVE)
					{
						referenceX = event.button.x;
						referenceY = event.button.y;
						initialX = initialY = 0;
						moving = true;
					}
					else if (mode == MODE_VERTEX)
					{
						x = float(event.button.x);
						y = float(event.button.y);

						grid.Snap(x, y);

						selection = FindSelection(map.GetSectors(), grid.TranslateXToGridSpace(float(event.button.x)), grid.TranslateYToGridSpace(float(event.button.y)), nullptr, &selectedLine, nullptr);

						if (selection == SELECTION_LINE)
						{
							Line *line = selectedLine->GetData();
							Vertex *newVertex = new Vertex;
							ProjectPointOnSegment(*line->vertex1->GetData(), *line->vertex2->GetData(), x, y, *newVertex);
							CNode<Vertex> *newVertexNode = map.GetVertices()->Insert(newVertex, true, line->vertex1);
							Line *newLine = new Line;
							newLine->vertex1 = line->vertex1;
							newLine->vertex2 = newVertexNode;
							newLine->sectors[0] = line->sectors[0];
							newLine->sectors[1] = line->sectors[1];
							line->vertex1 = newVertexNode;
							CNode<Line> *newLineNode = map.GetLines()->Insert(newLine, true, selectedLine->Prev());

							line->sectors[0]->vertexCount++;
							line->sectors[0]->lineCount++;

							if (selectedLine == line->sectors[0]->firstLine)
								line->sectors[0]->firstLine = newLineNode;
							else if (selectedLine == line->sectors[0]->lastLine)
								line->sectors[0]->lastVertex = newVertexNode;

							if (line->sectors[1] != nullptr)
							{
								CNode<Vertex> *currentVertex = line->sectors[1]->firstVertex;

								for (unsigned int vertexCount = line->sectors[1]->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
								{
									if (currentVertex->GetData() == line->vertex2->GetData())
									{
										newVertexNode = map.GetVertices()->Insert(newVertexNode, currentVertex);

										break;
									}
								}

								CNode<Line> *currentLine = line->sectors[1]->firstLine;

								for (unsigned int lineCount = line->sectors[1]->lineCount; lineCount-- != 0; currentLine = currentLine->Next())
								{
									if (currentLine->GetData() == line)
									{
										newLineNode = map.GetLines()->Insert(newLineNode, currentLine);

										if (currentLine == line->sectors[1]->lastLine)
										{
											line->sectors[1]->lastLine = newLineNode;
											line->sectors[1]->lastVertex = newVertexNode;
										}

										break;
									}
								}

								line->sectors[1]->vertexCount++;
								line->sectors[1]->lineCount++;
							}
						}
					}
				}
				else if (event.button.button == SDL_BUTTON_RIGHT && !drawing && !moving)
				{
					referenceX = event.button.x;
					referenceY = event.button.y;
					initialX = initialY = 0;
					scrolling = true;
				}

				break;
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT && !scrolling)
				{
					if (drawing)
					{
						Vertex vertex;

						x = float(event.button.x);
						y = float(event.button.y);

						grid.Snap(x, y);

						vertex.x = x;
						vertex.y = y;

						if (vertex.x == map.GetVertices()->Tail()->GetData()->x && vertex.y == map.GetVertices()->Tail()->GetData()->y)
							break;

						if (sector.vertexCount > 2 && AABBContainsPoint(vertex.x, vertex.y, sector.firstVertex->GetData()->x - 2, sector.firstVertex->GetData()->y - 2, sector.firstVertex->GetData()->x + 2, sector.firstVertex->GetData()->y + 2))
						{
							CloseSector(map, sector, line);
							InitializeSector(sector);
							drawing = false;
						}
						else
						{
							line.vertex2 = InsertVertex(map, vertex);

							if (line.vertex2->GetData()->x < sector.minX)
								sector.minX = line.vertex2->GetData()->x;

							if (line.vertex2->GetData()->y < sector.minY)
								sector.minY = line.vertex2->GetData()->y;

							if (line.vertex2->GetData()->x > sector.maxX)
								sector.maxX = line.vertex2->GetData()->x;

							if (line.vertex2->GetData()->y > sector.maxY)
								sector.maxY = line.vertex2->GetData()->y;

							if ((++sector.vertexCount % 2) == 0)
							{
								CNode<Line> *newLineNode = InsertLine(map, line);

								if (sector.lineCount++ == 0)
									sector.firstLine = sector.lastLine = newLineNode;
								else
									sector.lastLine = newLineNode;

								sector.vertexCount++;
							}

							line.vertex1 = line.vertex2;

							if (sector.vertexCount == 1)
								sector.firstVertex = sector.lastVertex = line.vertex1;
							else
								sector.lastVertex = line.vertex1;

							drawing = true;
						}
					}
					else if (moving)
					{
						if (selection == SELECTION_VERTEX)
							RecalculateSectorsAABB(map, *selectedVertex);
						else if (selection == SELECTION_LINE)
							RecalculateSectorsAABB(map, *selectedLine);
						else if (selection == SELECTION_SECTOR)
							RecalculateSectorsAABB(map, *selectedSector);

						moving = false;
					}
				}
				else if (event.button.button == SDL_BUTTON_RIGHT && scrolling)
					scrolling = false;

				break;
			case SDL_MOUSEMOTION:
				if (drawing)
				{
					x = float(event.motion.x);
					y = float(event.motion.y);

					grid.Snap(x, y);
				}
				else if (moving)
				{
					if (selection == SELECTION_VERTEX)
						MoveVertex(*selectedVertex->GetData(), event.motion.x, event.motion.y, grid);
					else if (selection == SELECTION_LINE)
						MoveLine(*selectedLine->GetData(), event.motion.x, event.motion.y, referenceX, referenceY, initialX, initialY, scaleInverse, grid);
					else if (selection == SELECTION_SECTOR)
						MoveSector(*selectedSector->GetData(), event.motion.x, event.motion.y, referenceX, referenceY, initialX, initialY, scaleInverse, grid);
				}
				else if (scrolling)
				{
					int finalX = event.motion.x - referenceX, finalY = event.motion.y - referenceY;
					grid.Scroll(finalX - initialX, finalY - initialY);
					initialX = finalX;
					initialY = finalY;
				}
				else if (mode == MODE_MOVE)
					selection = FindSelection(map.GetSectors(), grid.TranslateXToGridSpace(float(event.motion.x)), grid.TranslateYToGridSpace(float(event.motion.y)), &selectedSector, &selectedLine, &selectedVertex);

				break;
			case SDL_MOUSEWHEEL:
				if (event.wheel.y < 0 && scale > 0.25f)
					scale -= 0.25f;
				else if (event.wheel.y > 0 && scale < 2.0f)
					scale += 0.25f;

				scaleInverse = 1.0f / scale;

				grid.SetScale(scale);

				updateTitle = true;

				break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					grid.Resize(event.window.data1, event.window.data2);

				break;
			default:
				break;
			}
		}

		if (updateTitle)
		{
			string title = "Doom RPG Edit - Mode: " + (mode == MODE_DRAW ? string("Draw") : (mode == MODE_MOVE ? string("Move") : string("Vertex"))) + " - Zoom: " + to_string(int(scale * 100)) + "%";
			SDL_SetWindowTitle(window, title.c_str());
			updateTitle = false;
		}

		grid.Render(renderer);
		map.Render(renderer, grid);

		if (drawing)
		{
			const Vertex *vertex = line.vertex1->GetData();
			int x1 = int(grid.TranslateXToViewSpace(vertex->x));
			int y1 = int(grid.TranslateYToViewSpace(vertex->y));
			int x2 = int(grid.TranslateXToViewSpace(x));
			int y2 = int(grid.TranslateYToViewSpace(y));
			vector<SDL_Rect> rects;
			rects.push_back({ x1 - 2, y1 - 2, 5, 5 });
			rects.push_back({ x2 - 2, y2 - 2, 5, 5 });

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDrawLine(renderer, x1, y1, x2, y2);

			SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
			SDL_RenderFillRects(renderer, rects.data(), rects.size());
		}

		
		if (mode == MODE_MOVE && selection != SELECTION_NONE)
		{
			if (selection == SELECTION_VERTEX)
			{
				SDL_Rect rect = { int(grid.TranslateXToViewSpace(selectedVertex->GetData()->x)) - 2, int(grid.TranslateYToViewSpace(selectedVertex->GetData()->y)) - 2, 5, 5 };

				SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
			else if (selection == SELECTION_LINE)
			{
				const Vertex *vertex1 = selectedLine->GetData()->vertex1->GetData();
				const Vertex *vertex2 = selectedLine->GetData()->vertex2->GetData();
				SDL_Rect rects[2] = { { int(grid.TranslateXToViewSpace(vertex1->x)) - 2, int(grid.TranslateYToViewSpace(vertex1->y)) - 2, 5, 5 }, { int(grid.TranslateXToViewSpace(vertex2->x)) - 2, int(grid.TranslateYToViewSpace(vertex2->y)) - 2, 5, 5 } };

				SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);
				SDL_RenderDrawLine(renderer, int(grid.TranslateXToViewSpace(vertex1->x)), int(grid.TranslateYToViewSpace(vertex1->y)), int(grid.TranslateXToViewSpace(vertex2->x)), int(grid.TranslateYToViewSpace(vertex2->y)));
				
				SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
				SDL_RenderFillRects(renderer, rects, 2);
			}
			else if (selection == SELECTION_SECTOR)
			{
				vector<SDL_Point> screenCoords;
				vector<SDL_Rect> rects;
				CNode<Line> *currentLine = selectedSector->GetData()->firstLine;

				for (unsigned int lineCount = selectedSector->GetData()->lineCount; lineCount-- != 0; currentLine = currentLine->Next())
				{
					const Vertex *vertex = (currentLine->GetData()->sectors[0] == selectedSector->GetData() ? currentLine->GetData()->vertex1->GetData() : currentLine->GetData()->vertex2->GetData());
					screenCoords.push_back({ int(grid.TranslateXToViewSpace(vertex->x)), int(grid.TranslateYToViewSpace(vertex->y)) });
					rects.push_back({ screenCoords.back().x - 2, screenCoords.back().y - 2, 5, 5 });
				}

				SDL_SetRenderDrawColor(renderer, 255, 128, 0, 255);
				SDL_RenderDrawLines(renderer, screenCoords.data(), screenCoords.size());
				SDL_RenderDrawLine(renderer, screenCoords.front().x, screenCoords.front().y, screenCoords.back().x, screenCoords.back().y);

				SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
				SDL_RenderFillRects(renderer, rects.data(), rects.size());
			}
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}

bool SectorIsClockwise(const Sector &sector)
{
	float sum = 0.0f;
	CNode<Vertex> *currentVertex = sector.firstVertex;

	for (unsigned int vertexCount = sector.vertexCount - 1; vertexCount-- != 0; currentVertex = currentVertex->Next())
	{
		const Vertex *vertex1 = currentVertex->GetData();
		const Vertex *vertex2 = currentVertex->Next()->GetData();
		sum += (vertex2->x - vertex1->x) * (vertex2->y + vertex1->y);
	}

	const Vertex *vertex1 = currentVertex->GetData();
	const Vertex *vertex2 = sector.firstVertex->GetData();
	sum += (vertex2->x - vertex1->x) * (vertex2->y + vertex1->y);

	return (sum < 0.0f);
}

bool AABBContainsPoint(float x, float y, float minX, float minY, float maxX, float maxY)
{
	return (x >= minX && y >= minY && x <= maxX && y <= maxY);
}

bool AABBContainsSegment(float x1, float y1, float x2, float y2, float minX, float minY, float maxX, float maxY)
{
	if ((x1 < minX && x2 < minX) || (y1 < minY && y2 < minY) || (x1 > maxX && x2 > maxX) || (y1 > maxY && y2 > maxY))
		return false;

	float m = (y2 - y1) / (x2 - x1);

	float y = m * (minX - x1) + y1;

	if (y >= minY && y <= maxY)
		return true;

	y = m * (maxX - x1) + y1;
	
	if (y >= minY && y < maxY)
		return true;

	float x = (minY - y1) / m + x1;
	
	if (x >= minX && x <= maxX)
		return true;

	x = (maxY - y1) / m + x1;

	if (x >= minX && x <= maxX)
		return true;

	return false;
}

bool SectorContainsPoint(const Sector &sector, float x, float y)
{
	bool oddNodes = false;
	CNode<Line> *currentLine = sector.firstLine;

	for (unsigned int lineCount = sector.lineCount; lineCount-- != 0; currentLine = currentLine->Next())
	{
		const Vertex *vertex1 = currentLine->GetData()->vertex1->GetData();
		const Vertex *vertex2 = currentLine->GetData()->vertex2->GetData();

		if ((vertex1->y < y && vertex2->y >= y || vertex2->y < y && vertex1->y >= y) && (vertex1->x <= x || vertex2->x <= x))
			oddNodes ^= (vertex1->x + (y - vertex1->y) / (vertex2->y - vertex1->y) * (vertex2->x - vertex1->x) < x);
	}

	return oddNodes;
}

Selection FindSelection(CList<Sector> *sectors, float x, float y, CNode<Sector> **selectedSector, CNode<Line> **selectedLine, CNode<Vertex> **selectedVertex)
{
	if (selectedSector == nullptr && selectedLine == nullptr && selectedVertex == nullptr)
		return SELECTION_NONE;

	for (CNode<Sector> *currentSector = sectors->Head(); currentSector->GetData() != nullptr; currentSector = currentSector->Next())
	{
		if (!AABBContainsPoint(x, y, currentSector->GetData()->minX - 3, currentSector->GetData()->minY - 3, currentSector->GetData()->maxX + 3, currentSector->GetData()->maxY + 3))
			continue;

		CNode<Line> *currentLine = currentSector->GetData()->firstLine;

		for (unsigned int lineCount = currentSector->GetData()->lineCount; lineCount-- != 0; currentLine = currentLine->Next())
		{
			const Vertex *vertex1 = currentLine->GetData()->vertex1->GetData();
			const Vertex *vertex2 = currentLine->GetData()->vertex2->GetData();

			if (selectedVertex != nullptr && AABBContainsPoint(x, y, vertex1->x - 2, vertex1->y - 2, vertex1->x + 2, vertex1->y + 2))
			{
				*selectedVertex = currentLine->GetData()->vertex1;
				return SELECTION_VERTEX;
			}

			if (selectedVertex != nullptr && AABBContainsPoint(x, y, vertex2->x - 2, vertex2->y - 2, vertex2->x + 2, vertex2->y + 2))
			{
				*selectedVertex = currentLine->GetData()->vertex2;
				return SELECTION_VERTEX;
			}

			if (selectedLine != nullptr && AABBContainsSegment(vertex1->x, vertex1->y, vertex2->x, vertex2->y, x - 2, y - 2, x + 2, y + 2))
			{
				*selectedLine = currentLine;
				return SELECTION_LINE;
			}
		}

		if (selectedSector != nullptr && SectorContainsPoint(*currentSector->GetData(), x, y))
		{
			*selectedSector = currentSector;
			return SELECTION_SECTOR;
		}
	}

	return SELECTION_NONE;
}

CNode<Line> *FindLine(const CList<Line> *lines, const Vertex *vertex1, const Vertex *vertex2)
{
	for (CNode<Line> *currentLine = lines->Head(); currentLine->GetData() != nullptr; currentLine = currentLine->Next())
	{
		const Line *line = currentLine->GetData();

		if ((line->vertex1->GetData() == vertex1 && line->vertex2->GetData() == vertex2) || (line->vertex1->GetData() == vertex2 && line->vertex2->GetData() == vertex1))
			return currentLine;
	}

	return nullptr;
}

void ProjectPointOnSegment(const Vertex &vertex1, const Vertex &vertex2, float x, float y, Vertex &vertexOut)
{
	float t = ((vertex2.x - vertex1.x) * (x - vertex1.x) + (vertex2.y - vertex1.y) * (y - vertex1.y)) / ((vertex2.x - vertex1.x) * (vertex2.x - vertex1.x) + (vertex2.y - vertex1.y) * (vertex2.y - vertex1.y));
	vertexOut.x = vertex1.x + (vertex2.x - vertex1.x) * t;
	vertexOut.y = vertex1.y + (vertex2.y - vertex1.y) * t;
}

void CalculateSectorAABB(Sector &sector)
{
	sector.minX = sector.minY = FLT_MAX;
	sector.maxX = sector.maxY = -FLT_MAX;
	CNode<Vertex> *currentVertex = sector.firstVertex;

	for (unsigned int vertexCount = sector.vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
	{
		if (currentVertex->GetData()->x < sector.minX)
			sector.minX = currentVertex->GetData()->x;

		if (currentVertex->GetData()->y < sector.minY)
			sector.minY = currentVertex->GetData()->y;

		if (currentVertex->GetData()->x > sector.maxX)
			sector.maxX = currentVertex->GetData()->x;

		if (currentVertex->GetData()->y > sector.maxY)
			sector.maxY = currentVertex->GetData()->y;
	}
}

void InitializeSector(Sector &sector)
{
	sector = Sector();
	sector.vertexCount = 0;
	sector.lineCount = 0;
	sector.minX = sector.minY = FLT_MAX;
	sector.maxX = sector.maxY = -FLT_MAX;
}

void CancelSector(CMap &map, Sector &sector)
{
	if (sector.lineCount > 0)
	{
		map.GetVertices()->Delete(sector.firstVertex, sector.lastVertex->Next());
		map.GetLines()->Delete(sector.firstLine, sector.lastLine->Next());
	}
	else
		map.GetVertices()->Delete(sector.firstVertex);
}

void DeleteSector(CMap &map, CNode<Sector> &sector)
{
	CNode<Line> *currentLine = sector.GetData()->firstLine;

	for (unsigned int lineCount = sector.GetData()->lineCount; lineCount-- != 0; currentLine = currentLine->Next())
	{
		if (currentLine->GetRefCount() == 2)
		{
			if (currentLine->GetData()->sectors[0] == sector.GetData())
			{
				CNode<Vertex> *currentVertex = currentLine->GetData()->sectors[1]->firstVertex;

				for (unsigned int vertexCount = currentLine->GetData()->sectors[1]->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
				{
					if (currentVertex->GetData() == currentLine->GetData()->vertex2->GetData())
					{
						currentLine->GetData()->vertex1 = currentVertex;
						currentLine->GetData()->vertex2 = (vertexCount != 0 ? currentVertex->Next() : currentLine->GetData()->sectors[1]->firstVertex);

						break;
					}
				}

				currentLine->GetData()->sectors[0] = currentLine->GetData()->sectors[1];
				currentLine->GetData()->sectors[1] = nullptr;
			}
			else
				currentLine->GetData()->sectors[1] = nullptr;
		}
	}

	map.GetVertices()->Delete(sector.GetData()->firstVertex, sector.GetData()->lastVertex->Next());
	map.GetLines()->Delete(sector.GetData()->firstLine, sector.GetData()->lastLine->Next());
	map.GetSectors()->Delete(&sector);
}

CNode<Vertex> *InsertVertex(CMap &map, Vertex &vertex)
{
	CNode<Vertex> *selectedVertex = nullptr;
	CNode<Line> *selectedLine = nullptr;
	Selection selection = FindSelection(map.GetSectors(), vertex.x, vertex.y, nullptr, &selectedLine, &selectedVertex);

	if (selection == SELECTION_VERTEX)
		return map.GetVertices()->Insert(selectedVertex);
	else if (selection == SELECTION_LINE)
	{
		Line *line = selectedLine->GetData();
		Vertex *newVertex = new Vertex;
		ProjectPointOnSegment(*line->vertex1->GetData(), *line->vertex2->GetData(), vertex.x, vertex.y, *newVertex);
		CNode<Vertex> *newVertexNode = map.GetVertices()->Insert(newVertex, true, line->vertex1);
		Line *newLine = new Line;
		newLine->vertex1 = line->vertex1;
		newLine->vertex2 = newVertexNode;
		newLine->sectors[0] = line->sectors[0];
		newLine->sectors[1] = line->sectors[1];
		line->vertex1 = newVertexNode;
		CNode<Line> *newLineNode = map.GetLines()->Insert(newLine, true, selectedLine->Prev());

		line->sectors[0]->vertexCount++;
		line->sectors[0]->lineCount++;

		if (selectedLine == line->sectors[0]->firstLine)
			line->sectors[0]->firstLine = newLineNode;
		else if (selectedLine == line->sectors[0]->lastLine)
			line->sectors[0]->lastVertex = newVertexNode;

		return map.GetVertices()->Insert(newVertexNode);
	}
	else
		return map.GetVertices()->Insert(new Vertex(vertex));
}

CNode<Line> *InsertLine(CMap &map, Line &line)
{
	CNode<Line> *newLineNode;

	if (line.vertex1->GetRefCount() > 1 && line.vertex2->GetRefCount() > 1)
	{
		CNode<Line> *refLineNode = FindLine(map.GetLines(), line.vertex1->GetData(), line.vertex2->GetData());

		if (refLineNode != nullptr)
			newLineNode = map.GetLines()->Insert(refLineNode);
		else
			newLineNode = map.GetLines()->Insert(new Line(line));
	}
	else
		newLineNode = map.GetLines()->Insert(new Line(line));

	return newLineNode;
}

CNode<Sector> *InsertSector(CMap &map, Sector &sector)
{
	if (!SectorIsClockwise(sector))
	{
		CNode<Vertex> *tempVertexNode = sector.firstVertex->Next();

		map.GetVertices()->Reverse(sector.firstVertex->Next(), sector.lastVertex->Next());
		map.GetLines()->Reverse(sector.firstLine, sector.lastLine->Next());

		sector.lastVertex = tempVertexNode;

		CNode<Line> *tempLineNode = sector.firstLine;
		sector.firstLine = sector.lastLine;
		sector.lastLine = tempLineNode;

		CNode<Line> *currentLine = sector.firstLine;

		for (unsigned int lineCount = sector.lineCount; lineCount-- != 0; currentLine = currentLine->Next())
		{
			if (currentLine->GetRefCount() == 1)
			{
				tempVertexNode = currentLine->GetData()->vertex1;
				currentLine->GetData()->vertex1 = currentLine->GetData()->vertex2;
				currentLine->GetData()->vertex2 = tempVertexNode;
			}
		}
	}

	Sector *newSector = new Sector(sector);

	CNode<Line> *currentLine = sector.firstLine;

	for (unsigned int lineCount = sector.lineCount; lineCount-- != 0; currentLine = currentLine->Next())
	{
		if (currentLine->GetRefCount() == 1)
			currentLine->GetData()->sectors[0] = newSector;
		else
			currentLine->GetData()->sectors[1] = newSector;
	}

	return map.GetSectors()->Insert(newSector);
}

void CloseSector(CMap &map, Sector &sector, Line &line)
{
	line.vertex2 = sector.firstVertex;

	sector.vertexCount = (sector.vertexCount + 1) / 2;
	sector.lineCount++;
	sector.lastLine = InsertLine(map, line);

	InsertSector(map, sector);
}

void MoveVertex(Vertex &vertex, int x, int y, CGrid &grid)
{
	vertex.x = float(x);
	vertex.y = float(y);

	grid.Snap(vertex.x, vertex.y);
}

void MoveLine(Line &line, int x, int y, int referenceX, int referenceY, int &initialX, int &initialY, float scaleInverse, CGrid &grid)
{
	int finalX = (x - referenceX) / grid.GetScaledCellSize() * grid.GetScaledCellSize();
	int finalY = (y - referenceY) / grid.GetScaledCellSize() * grid.GetScaledCellSize();

	int xDisplacement = finalX - initialX, yDisplacement = finalY - initialY;

	line.vertex1->GetData()->x += xDisplacement * scaleInverse;
	line.vertex1->GetData()->y += yDisplacement * scaleInverse;
	line.vertex2->GetData()->x += xDisplacement * scaleInverse;
	line.vertex2->GetData()->y += yDisplacement * scaleInverse;

	initialX = finalX;
	initialY = finalY;
}

void MoveSector(Sector &sector, int x, int y, int referenceX, int referenceY, int &initialX, int &initialY, float scaleInverse, CGrid &grid)
{
	int finalX = (x - referenceX) / grid.GetScaledCellSize() * grid.GetScaledCellSize();
	int finalY = (y - referenceY) / grid.GetScaledCellSize() * grid.GetScaledCellSize();

	int xDisplacement = finalX - initialX, yDisplacement = finalY - initialY;

	CNode<Line> *currentLine = sector.firstLine;

	for (unsigned int lineCount = sector.lineCount; lineCount-- != 0; currentLine = currentLine->Next())
	{
		Vertex *vertex = (currentLine->GetData()->sectors[0] == &sector ? currentLine->GetData()->vertex1->GetData() : currentLine->GetData()->vertex2->GetData());
		vertex->x += xDisplacement * scaleInverse;
		vertex->y += yDisplacement * scaleInverse;
	}

	sector.minX += xDisplacement * scaleInverse;
	sector.minY += yDisplacement * scaleInverse;
	sector.maxX += xDisplacement * scaleInverse;
	sector.maxY += yDisplacement * scaleInverse;

	initialX = finalX;
	initialY = finalY;
}

void RecalculateSectorsAABB(CMap &map, CNode<Vertex> &vertex)
{
	unsigned int refCount = vertex.GetRefCount();
	bool allReferencesFound = false;

	for (CNode<Sector> *currentSector = map.GetSectors()->Head(); currentSector->GetData() != nullptr; currentSector = currentSector->Next())
	{
		CNode<Vertex> *currentVertex = currentSector->GetData()->firstVertex;

		for (unsigned int vertexCount = currentSector->GetData()->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
		{
			if (currentVertex->GetData() == vertex.GetData())
			{
				allReferencesFound = (--refCount == 0);

				CalculateSectorAABB(*currentSector->GetData());

				break;
			}
		}

		if (allReferencesFound)
			break;
	}
}

void RecalculateSectorsAABB(CMap &map, CNode<Line> &line)
{
	unsigned int refCount = line.GetData()->vertex1->GetRefCount() + line.GetData()->vertex2->GetRefCount();
	bool allReferencesFound = false;

	for (CNode<Sector> *currentSector = map.GetSectors()->Head(); currentSector->GetData() != nullptr; currentSector = currentSector->Next())
	{
		CNode<Vertex> *currentVertex = currentSector->GetData()->firstVertex;

		for (unsigned int vertexCount = currentSector->GetData()->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
		{
			if (currentVertex->GetData() == line.GetData()->vertex1->GetData())
			{
				allReferencesFound = (--refCount == 0);

				if ((currentVertex != currentSector->GetData()->lastVertex && currentVertex->Next()->GetData() == line.GetData()->vertex2->GetData()) ||
					(currentSector->GetData()->firstVertex->GetData() == line.GetData()->vertex2->GetData()) ||
					(currentVertex != currentSector->GetData()->firstVertex && currentVertex->Prev()->GetData() == line.GetData()->vertex2->GetData()) ||
					(currentSector->GetData()->lastVertex->GetData() == line.GetData()->vertex2->GetData()))
					allReferencesFound = (--refCount == 0);

				CalculateSectorAABB(*currentSector->GetData());

				break;
			}
			else if (currentVertex->GetData() == line.GetData()->vertex2->GetData())
			{
				allReferencesFound = (--refCount == 0);

				if ((currentVertex != currentSector->GetData()->lastVertex && currentVertex->Next()->GetData() == line.GetData()->vertex1->GetData()) ||
					(currentSector->GetData()->firstVertex->GetData() == line.GetData()->vertex1->GetData()) ||
					(currentVertex != currentSector->GetData()->firstVertex && currentVertex->Prev()->GetData() == line.GetData()->vertex1->GetData()) ||
					(currentSector->GetData()->lastVertex->GetData() == line.GetData()->vertex1->GetData()))
					allReferencesFound = (--refCount == 0);

				CalculateSectorAABB(*currentSector->GetData());

				break;
			}
		}

		if (allReferencesFound)
			break;
	}
}

void RecalculateSectorsAABB(CMap &map, CNode<Sector> &sector)
{
	unsigned int refCount = 0;
	bool allReferencesFound = false;
	bool sectorReferencesVertices = false;
	CNode<Vertex> *currentVertex = sector.GetData()->firstVertex;

	for (unsigned int vertexCount = sector.GetData()->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
		refCount += currentVertex->GetRefCount() - 1;

	if (refCount > 0)
	{
		for (CNode<Sector> *currentSector = map.GetSectors()->Head(); sector.GetData() != nullptr; currentSector = currentSector->Next())
		{
			if (currentSector == &sector)
				continue;

			CNode<Vertex> *currentVertex = currentSector->GetData()->firstVertex;

			for (unsigned int vertexCount = currentSector->GetData()->vertexCount; vertexCount-- != 0; currentVertex = currentVertex->Next())
			{
				CNode<Vertex> *currentSelectedVertex = sector.GetData()->firstVertex;

				for (unsigned int selectedVertexCount = sector.GetData()->vertexCount; selectedVertexCount-- != 0; currentSelectedVertex = currentSelectedVertex->Next())
				{
					if (currentVertex->GetData() == currentSelectedVertex->GetData())
					{
						allReferencesFound = (--refCount == 0);
						sectorReferencesVertices = true;

						if (allReferencesFound)
							break;
					}
				}

				if (allReferencesFound)
					break;
			}

			if (sectorReferencesVertices)
			{
				CalculateSectorAABB(*currentSector->GetData());
				sectorReferencesVertices = false;
			}

			if (allReferencesFound)
				break;
		}
	}
}