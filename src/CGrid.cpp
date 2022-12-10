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

#include <algorithm>

#include "CGrid.h"

void CGrid::Resize(int width, int height)
{
	m_gridWidth = m_viewWidth = width;
	m_gridHeight = m_viewHeight = height;
	m_xSize = m_viewWidth / m_scaledCellSize - 1;
	m_ySize = m_viewHeight / m_scaledCellSize - 1;

	if ((m_viewWidth % m_scaledCellSize) != 0)
	{
		m_xSize++;
		m_gridWidth = m_xSize * m_scaledCellSize + m_scaledCellSize;
	}

	if ((m_viewHeight % m_scaledCellSize) != 0)
	{
		m_ySize++;
		m_gridHeight = m_ySize * m_scaledCellSize + m_scaledCellSize;
	}
}

void CGrid::Render(SDL_Renderer *renderer)
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);

	int minBounds[2];
	int maxBounds[2];

	minBounds[0] = m_minX * m_scaledCellSize;
	minBounds[1] = m_minY * m_scaledCellSize;

	maxBounds[0] = m_maxX * m_scaledCellSize;
	maxBounds[1] = m_maxY * m_scaledCellSize;

	int xStart = std::max(minBounds[0] + m_originX + m_xDisplacement, 0);
	int yStart = std::max(minBounds[1] + m_originY + m_yDisplacement, 0);

	int xEnd = std::min(maxBounds[0] + m_originX + m_xDisplacement, m_gridWidth - 1);
	int yEnd = std::min(maxBounds[1] + m_originY + m_yDisplacement, m_gridHeight - 1);

	for (int x = 0; x <= m_xSize; x++)
	{
		int xCoord = x * m_scaledCellSize + m_scaledCellSize / 2 + m_xDisplacement;
		
		if (xCoord < 0)
			xCoord = m_gridWidth + (xCoord % m_gridWidth);
		else if (xCoord >= m_gridWidth)
			xCoord %= m_gridWidth;

		if (xCoord - m_originX - m_xDisplacement < minBounds[0])
			continue;
		else if (xCoord - m_originX - m_xDisplacement > maxBounds[0])
			continue;

		SDL_RenderDrawLine(renderer, xCoord, yStart, xCoord, yEnd);
	}

	for (int y = 0; y <= m_ySize; y++)
	{
		int yCoord = y * m_scaledCellSize + m_scaledCellSize / 2 + m_yDisplacement;

		if (yCoord < 0)
			yCoord = m_gridHeight + (yCoord % m_gridHeight);
		else if (yCoord >= m_gridHeight)
			yCoord %= m_gridHeight;

		if (yCoord - m_originY - m_yDisplacement < minBounds[1])
			continue;
		else if (yCoord - m_originY - m_yDisplacement > maxBounds[1])
			continue;

		SDL_RenderDrawLine(renderer, xStart, yCoord, xEnd, yCoord);
	}
}

void CGrid::Snap(float &x, float &y)
{
	x -= m_xDisplacement;

	if (x < 0)
		x = float(int(x) / m_scaledCellSize * m_scaledCellSize - m_scaledCellSize / 2 - m_originX);
	else
		x = float(int(x) / m_scaledCellSize * m_scaledCellSize + m_scaledCellSize / 2 - m_originX);

	x = x * m_scaleInverse;

	y -= m_yDisplacement;

	if (y < 0)
		y = float(int(y) / m_scaledCellSize * m_scaledCellSize - m_scaledCellSize / 2 - m_originY);
	else
		y = float(int(y) / m_scaledCellSize * m_scaledCellSize + m_scaledCellSize / 2 - m_originY);

	y = y * m_scaleInverse;
}

void CGrid::TranslateToGridSpace(float &x, float &y)
{
	x = (x - m_originX - m_xDisplacement) * m_scaleInverse;
	y = (y - m_originY + m_yDisplacement) * m_scaleInverse;
}

void CGrid::TranslateToViewSpace(float &x, float &y)
{
	x = x * m_scale + m_originX + m_xDisplacement;
	y = y * m_scale + m_originY + m_yDisplacement;
}

float CGrid::SnapX(float x)
{
	x -= m_xDisplacement;

	if (x < 0)
		x = float(int(x) / m_scaledCellSize * m_scaledCellSize - m_scaledCellSize / 2 - m_originX);
	else
		x = float(int(x) / m_scaledCellSize * m_scaledCellSize + m_scaledCellSize / 2 - m_originX);

	return (x * m_scaleInverse);
}

float CGrid::SnapY(float y)
{
	y -= m_yDisplacement;

	if (y < 0)
		y = float(int(y) / m_scaledCellSize * m_scaledCellSize - m_scaledCellSize / 2 - m_originY);
	else
		y = float(int(y) / m_scaledCellSize * m_scaledCellSize + m_scaledCellSize / 2 - m_originY);

	return (y * m_scaleInverse);
}

float CGrid::TranslateXToGridSpace(float x)
{
	return ((x - m_originX - m_xDisplacement) * m_scaleInverse);
}

float CGrid::TranslateYToGridSpace(float y)
{
	return ((y - m_originY - m_yDisplacement) * m_scaleInverse);
}

float CGrid::TranslateXToViewSpace(float x)
{
	return (x * m_scale + m_originX + m_xDisplacement);
}

float CGrid::TranslateYToViewSpace(float y)
{
	return (y * m_scale + m_originY + m_yDisplacement);
}