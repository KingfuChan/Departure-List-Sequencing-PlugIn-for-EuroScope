#pragma once
//#pragma warning(disable:4996) // ignore strcpy_s and others

#include <iostream>
#include <string>
#include <EuroScopePlugIn.h>


struct SequenceData {
	CString m_callsign;
	int m_status;
	bool m_active;
};

struct SequencePosition {
	int m_index; // 
	int m_sequence;
};

class CSequencingPlugIn :
	public EuroScopePlugIn::CPlugIn
{
public:
	CSequencingPlugIn(void);
	~CSequencingPlugIn(void);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual bool OnCompileCommand(const char* sCommandLine);
	virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnTimer(int Counter);

private:
	CArray<SequenceData, SequenceData&> m_SequenceArray;
	bool CustomColorStby, CustomColorClrd;
	COLORREF CurrentColorStby, CurrentColorClrd; // do not use when bool above are false
	int CustomRefreshIntv, CustomMaxSpeed;
	SequencePosition GetSequenceData(const char* callsign);
	bool IsCallsignOnline(const char* callsign);
	bool ParseColorFromText(const char* text, COLORREF& color);
	void DisplayMessage(const char* msg);
};

