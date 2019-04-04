#pragma once

class MyMemDC : public CDC
{
public:

	// constructor sets up the memory DC
	MyMemDC(CDC* pDC);

	// Destructor copies the contents of the mem DC to the original DC
	~MyMemDC();

	// Allow usage as a pointer
	MyMemDC* operator->() { return this; }

	// Allow usage as a pointer
	operator MyMemDC*() { return this; }

private:
	CBitmap  m_bitmap;      // Offscreen bitmap
	CBitmap* m_pOldBitmap;  // bitmap originally found in MyMemDC
	CDC*     m_pDC;         // Saves CDC passed in constructor
	CRect    m_rect;        // Rectangle of drawing area.
	BOOL     m_bMemDC;      // TRUE if CDC really is a Memory DC.
};