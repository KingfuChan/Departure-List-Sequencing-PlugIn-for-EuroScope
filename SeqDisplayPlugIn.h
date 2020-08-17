#pragma once

#include <iostream>
#include "SeqDataPlugIn.h"


class SeqDisplayPlugIn :
	public SeqDataPlugIn
{
public:
	SeqDisplayPlugIn(void);
	~SeqDisplayPlugIn(void);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual bool OnCompileCommand(const char* sCommandLine);
	virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);

private:
	bool CustomColorStby, CustomColorClrd;
	COLORREF CurrentColorStby, CurrentColorClrd; // do not use when bool above are false
	bool ParseColorFromText(const char* text, COLORREF& color);
	void DisplayMessage(const char* msg);
};

