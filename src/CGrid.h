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

#ifndef __CGRID_H__
#define __CGRID_H__

#include <climits>

#include "SDL.h"

class CGrid
{
public:
	CGrid(int width, int height, int cellSize = 16, int xDisplacement = 0, int yDisplacement = 0, float scale = 1.0f, int minX = SHRT_MIN, int minY = SHRT_MIN, int maxX = SHRT_MAX, int maxY = SHRT_MAX) : m_cellSize(cellSize), m_scaledCellSize(int(cellSize * scale)), m_xDisplacement(xDisplacement), m_yDisplacement(yDisplacement), m_scale(scale), m_scaleInverse(1.0f / scale), m_minX(minX), m_minY(minY), m_maxX(maxX), m_maxY(maxY) { Resize(width, height); m_originX = m_xSize / 2 * m_scaledCellSize + m_scaledCellSize / 2; m_originY = m_ySize / 2 * m_scaledCellSize + m_scaledCellSize / 2; }

	void Resize(int width, int height);

	void Render(SDL_Renderer *renderer);

	void Snap(float &x, float &y);
	void TranslateToGridSpace(float &x, float &y);
	void TranslateToViewSpace(float &x, float &y);
	float SnapX(float x);
	float SnapY(float y);
	float TranslateXToGridSpace(float x);
	float TranslateYToGridSpace(float y);
	float TranslateXToViewSpace(float x);
	float TranslateYToViewSpace(float y);

	void CenterOrigin() { m_originX = m_xSize / 2 * m_scaledCellSize + m_scaledCellSize / 2; m_originY = m_ySize / 2 * m_scaledCellSize + m_scaledCellSize / 2; m_xDisplacement = 0; m_yDisplacement = 0; }
	void Scroll(int xDisplacement, int yDisplacement) { m_xDisplacement += xDisplacement; m_yDisplacement += yDisplacement; }
	void ScrollX(int xDisplacement) { m_xDisplacement += xDisplacement; }
	void ScrollY(int yDisplacement) { m_yDisplacement += yDisplacement; }

	int GetCellSize() const { return m_cellSize; }
	void SetCellSize(int cellSize) { m_cellSize = cellSize; }

	int GetScaledCellSize() const { return m_scaledCellSize; }

	int GetXDisplacement() const { return m_xDisplacement; }
	void SetXDisplacement(int xDisplacement) { m_xDisplacement = xDisplacement; }

	int GetYDisplacement() const { return m_yDisplacement; }
	void SetYDisplacement(int yDisplacement) { m_yDisplacement = yDisplacement; }

	int GetMinX() const { return m_minX; }
	void SetMinX(int minX) { m_minX = minX; }

	int GetMinY() const { return m_minY; }
	void SetMinY(int minY) { m_minY = minY; }

	int GetMaxX() const { return m_maxX; }
	void SetMaxX(int maxX) { m_maxX = maxX; }

	int GetMaxY() const { return m_maxY; }
	void SetMaxY(int maxY) { m_maxY = maxY; }

	float GetScale() const { return m_scale; }
	void SetScale(float scale) { if (scale == 0.0f) return; m_scaledCellSize = int(m_cellSize * scale); m_scale = scale; m_scaleInverse = 1.0f / scale; Resize(m_viewWidth, m_viewHeight); m_originX = m_xSize / 2 * m_scaledCellSize + m_scaledCellSize / 2; m_originY = m_ySize / 2 * m_scaledCellSize + m_scaledCellSize / 2; }

private:
	int m_xSize;
	int m_ySize;
	int m_cellSize;
	int m_scaledCellSize;
	int m_gridWidth;
	int m_gridHeight;
	int m_viewWidth;
	int m_viewHeight;
	int m_originX;
	int m_originY;
	int m_xDisplacement;
	int m_yDisplacement;
	int m_minX;
	int m_minY;
	int m_maxX;
	int m_maxY;
	float m_scale;
	float m_scaleInverse;
};

#endif