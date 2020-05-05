#include "pch.h"
#include "SequencingPlugIn.h"


#define PLUGIN_NAME "Departure List Sequencing"
#define PLUGIN_VERSION "0.9.0"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "Still under development"


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_SEQ_STATUS = 1;

// TAG FUNCTIONS
const int TAG_FUNC_SEQ_POPUP = 1; // pop up item
const int TAG_FUNC_SEQ_START = 11;
const int TAG_FUNC_SEQ_RESET = 12; // reset status
const int TAG_FUNC_SEQ_NEXT = 13; // next status
const int TAG_FUNC_SEQ_PREV = 14; // previous status
const int TAG_FUNC_SEQ_EDIT = 15; // edit sequence
const int TAG_FUNC_SEQ_DELETE = 16; // delete from database

namespace GroundStatus { // note that even numbers are clrd
	const int STBY_CLEARANCE = 1;
	const int CLRD_CLEARANCE = 2;
	const int STBY_PUSHTAXI = 3;
	const int CLRD_PUSHTAXI = 4;
	const int STBY_TAKEOFF = 5;
	const int CLRD_TAKEOFF = 6;
};

using namespace EuroScopePlugIn;

CSequencingPlugIn::CSequencingPlugIn(void)
	: CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	//自己的初始化
	AddAlias(".cjf", "Kingfu Chan"); //just playing

	//tag related
	RegisterTagItemType("Ground Sequence", TAG_ITEM_TYPE_SEQ_STATUS);
	RegisterTagItemFunction("Popup Menu", TAG_FUNC_SEQ_POPUP);
	RegisterTagItemFunction("Start Sequencing", TAG_FUNC_SEQ_START);
	RegisterTagItemFunction("Reset Status", TAG_FUNC_SEQ_RESET);
	RegisterTagItemFunction("Next Status", TAG_FUNC_SEQ_NEXT);
	RegisterTagItemFunction("Previous Status", TAG_FUNC_SEQ_PREV);

}


CSequencingPlugIn::~CSequencingPlugIn(void) {
}


SequenceData* CSequencingPlugIn::GetSequenceData(const char* callsign, int& sequence) {
	//also calculates sequence number
	int i;

	for (i = sequence = 0; i <= ArrayDel.GetUpperBound(); i++) {
		if (ArrayDel[i].m_active && ArrayDel[i].m_status % 2)
			sequence++;
		if (ArrayDel[i].m_flightplan.GetCallsign() == callsign)
			return &ArrayDel[i];
	}

	for (i = sequence = 0; i <= ArrayPus.GetUpperBound(); i++) {
		if (ArrayPus[i].m_active && ArrayPus[i].m_status % 2)
			sequence++;
		if (ArrayPus[i].m_flightplan.GetCallsign() == callsign)
			return &ArrayPus[i];
	}

	for (i = sequence = 0; i <= ArrayOff.GetUpperBound(); i++) {
		if (ArrayOff[i].m_active && ArrayOff[i].m_status % 2)
			sequence++;
		if (ArrayOff[i].m_flightplan.GetCallsign() == callsign)
			return &ArrayOff[i];
	}

	return nullptr;
}

int CSequencingPlugIn::GetSequenceIndex(const char* callsign, FlightArray* array) {
	int i;
	for (i = 0; i <= array->GetUpperBound(); i++)
		if (array->GetAt(i).m_flightplan.GetCallsign() == callsign)
			return i;
	return -1; // unable to find
}

void CSequencingPlugIn::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget,
	int ItemCode, int TagData, char sItemString[16],
	int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!FlightPlan.IsValid())
		return;

	SequenceData* seqdat;
	int seq;
	const char DASHES[] = "--------"; // to solve th "buffer too small" problem

	if ((seqdat = GetSequenceData(FlightPlan.GetCallsign(), seq)) == nullptr) {
		strcpy_s(sItemString, strlen(DASHES) + 1, DASHES);
		return;
	}

	if (ItemCode == TAG_ITEM_TYPE_SEQ_STATUS) {
		char sts[4];

		switch (seqdat->m_status)
		{
		case GroundStatus::CLRD_CLEARANCE:
			seq = 0;
		case GroundStatus::STBY_CLEARANCE:
			strcpy_s(sts, "CLR");
			break;

		case GroundStatus::CLRD_PUSHTAXI:
			seq = 0;
		case GroundStatus::STBY_PUSHTAXI:
			strcpy_s(sts, "P/S");
			break;

		case GroundStatus::CLRD_TAKEOFF:
			seq = 0;
		case GroundStatus::STBY_TAKEOFF:
			strcpy_s(sts, "DEP");
			break;

		default:
			return;
		}

		sprintf_s(sItemString, strlen(DASHES) + 1, "%d-%s-%.2d", seqdat->m_status, sts, seq);
	}
}

void CSequencingPlugIn::OnFlightPlanFlightPlanDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan) {

}

void CSequencingPlugIn::OnFlightPlanControllerAssignedDataUpdate(EuroScopePlugIn::CFlightPlan FlightPlan, int DataType) {

}

void CSequencingPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	EuroScopePlugIn::CFlightPlan fp;
	SequenceData* seqdat = nullptr, seqnew;
	FlightArray* fa = nullptr;
	int seq, idx;

	fp = FlightPlanSelectASEL();
	if (!fp.IsValid())
		return;

	seqdat = GetSequenceData(fp.GetCallsign(), seq);

	if (seqdat == nullptr && FunctionId != TAG_FUNC_SEQ_POPUP && FunctionId != TAG_FUNC_SEQ_START)
		return;
	else if (seqdat != nullptr) { // sequence data valid, initialize array and index
		if ((idx = GetSequenceIndex(fp.GetCallsign(), &ArrayDel)) != -1)
			fa = &ArrayDel;
		else if ((idx = GetSequenceIndex(fp.GetCallsign(), &ArrayPus)) != -1)
			fa = &ArrayPus;
		else if ((idx = GetSequenceIndex(fp.GetCallsign(), &ArrayOff)) != -1)
			fa = &ArrayOff;
	}

	switch (FunctionId)
	{

	case TAG_FUNC_SEQ_POPUP:

		OpenPopupList(Area, "SEQ Menu", 2);

		if (seqdat == nullptr)
			AddPopupListElement("Start", "SEQ", TAG_FUNC_SEQ_START);
		else {
			AddPopupListElement("Next", "STS", TAG_FUNC_SEQ_NEXT);
			AddPopupListElement("Prev", "STS", TAG_FUNC_SEQ_PREV);
			AddPopupListElement("Edit", "SEQ", TAG_FUNC_SEQ_EDIT);
			AddPopupListElement("Reset", "STS", TAG_FUNC_SEQ_RESET);
			AddPopupListElement("Delete", "", TAG_FUNC_SEQ_DELETE);
		}

		break;
	case TAG_FUNC_SEQ_START:

		seqnew.m_flightplan = fp;
		seqnew.m_active = true;
		seqnew.m_status = GroundStatus::STBY_CLEARANCE;
		ArrayDel.Add(seqnew);

		break;
	case TAG_FUNC_SEQ_RESET:

		seqnew = fa->GetAt(idx);
		fa->RemoveAt(idx);
		seqnew.m_flightplan = fp;
		seqnew.m_status = GroundStatus::STBY_CLEARANCE;
		ArrayDel.Add(seqnew);

		break;
	case TAG_FUNC_SEQ_NEXT:

		seqnew = fa->GetAt(idx);
		switch (seqnew.m_status)
		{
		case GroundStatus::CLRD_CLEARANCE:
			seqnew.m_status++;
			fa->RemoveAt(idx);
			ArrayPus.Add(seqnew);
			break;
		case GroundStatus::CLRD_PUSHTAXI:
			seqnew.m_status++;
			fa->RemoveAt(idx);
			ArrayOff.Add(seqnew);
			break;
		case GroundStatus::CLRD_TAKEOFF:
			break;
		default:
			seqnew.m_status++;
			fa->SetAt(idx, seqnew);
			break;
		}

		break;
	case TAG_FUNC_SEQ_PREV:

		seqnew = fa->GetAt(idx);
		switch (seqnew.m_status)
		{
		case GroundStatus::STBY_CLEARANCE:
			break;
		case GroundStatus::STBY_PUSHTAXI:
			seqnew.m_status--;
			fa->RemoveAt(idx);
			ArrayDel.Add(seqnew);
			break;
		case GroundStatus::STBY_TAKEOFF:
			seqnew.m_status--;
			fa->RemoveAt(idx);
			ArrayPus.Add(seqnew);
			break;
		default:
			seqnew.m_status--;
			fa->SetAt(idx, seqnew);
			break;
		}

		break;
	case TAG_FUNC_SEQ_EDIT:
		break;
	case TAG_FUNC_SEQ_DELETE:

		fa->RemoveAt(idx);

		break;
	default:
		break;
	}
}

void CSequencingPlugIn::OnFlightPlanDisconnect(CFlightPlan FlightPlan) {
	SequenceData* seqdat;
	int _seq;
	seqdat = GetSequenceData(FlightPlan.GetCallsign(), _seq);
	if (seqdat != nullptr)
		seqdat->m_active = false;
}
