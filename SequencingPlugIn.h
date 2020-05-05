#pragma once
//#pragma warning(disable:4996) // ignore strcpy_s and others

#include <iostream>
#include <string>
#include <EuroScopePlugIn.h>


class SequenceData {
public:
	EuroScopePlugIn::CFlightPlan m_flightplan;
	int m_status;
	bool m_active;
};

typedef CArray<SequenceData, SequenceData&>FlightArray;

class CSequencingPlugIn :
	public EuroScopePlugIn::CPlugIn
{
public:
	CSequencingPlugIn(void);
	~CSequencingPlugIn(void);
	virtual void OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan);
	virtual void OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType);
	virtual void OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area);
	virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan, EuroScopePlugIn::CRadarTarget RadarTarget, int ItemCode, int TagData,
		char sItemString[16], int* pColorCode, COLORREF* pRGB, double* pFontSize);
	virtual void OnFlightPlanDisconnect(EuroScopePlugIn::CFlightPlan FlightPlan);

private:
	FlightArray ArrayDel; // delivery
	FlightArray ArrayPus; // pushback
	FlightArray ArrayOff; // takeoff
	SequenceData* GetSequenceData(const char* callsign, int& sequence);
	int GetSequenceIndex(const char* callsign, FlightArray* array);
};

