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
const int TAG_FUNC_SEQ_EDIT_POPUP = 2; // edit
const int TAG_FUNC_SEQ_START = 11;
const int TAG_FUNC_SEQ_RESET = 12; // reset status
const int TAG_FUNC_SEQ_NEXT = 13; // next status
const int TAG_FUNC_SEQ_PREV = 14; // previous status
const int TAG_FUNC_SEQ_EDIT_FINISH = 15; // finish edit popup
const int TAG_FUNC_SEQ_DELETE = 16; // delete from database

// arrray indexes, see .h file SequencePosition
const int M_ARRAY_NOMATCH = -1;

// colors
const COLORREF TAG_COLOR_GREY = 0x009B9B9B;
const COLORREF TAG_COLOR_BLUE = 0x00E2E481;

// tag texts
const char DASHES[] = "-------"; // place-holder
const char STATUS_DESCRIPTION[3][5] = { "CLRN","PUST","TKOF" };

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
	// custsom initialization
	AddAlias(".cjf", "Kingfu Chan"); //just for fun

	//tag related
	RegisterTagItemType("Ground Sequence", TAG_ITEM_TYPE_SEQ_STATUS);
	RegisterTagItemFunction("Popup Menu", TAG_FUNC_SEQ_POPUP);

	m_SequenceArray.RemoveAll();
}


CSequencingPlugIn::~CSequencingPlugIn(void) {
}


SequencePosition CSequencingPlugIn::GetSequenceData(const char* callsign) {
	//also calculates sequence number, note that may not be able to change data
	int ary, idx, seq[3] = { 0,0,0 };
	SequencePosition sp;

	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		ary = (m_SequenceArray[idx].m_status - 1) / 2; // array index
		if (m_SequenceArray[idx].m_active && m_SequenceArray[idx].m_status % 2) // stby status
			++seq[ary];
		if (!strcmp(m_SequenceArray[idx].m_callsign, callsign)) {
			sp.m_index = idx;
			sp.m_sequence = seq[ary];
			return sp;
		}
	}

	sp.m_index = M_ARRAY_NOMATCH;
	sp.m_sequence = -1;
	return sp;
}


bool CSequencingPlugIn::IsCallsignOnline(const char* callsign) {
	CRadarTarget rt;
	for (rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt))
		if (!strcmp(callsign, rt.GetCallsign()))
			return true;
	return false;
}

void CSequencingPlugIn::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget,
	int ItemCode, int TagData, char sItemString[16],
	int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!(FlightPlan.IsValid() || RadarTarget.IsValid()))
		return;

	if (ItemCode == TAG_ITEM_TYPE_SEQ_STATUS) {
		SequencePosition seqpos;

		seqpos = GetSequenceData(FlightPlan.GetCallsign());
		if (seqpos.m_index == M_ARRAY_NOMATCH) {
			strcpy_s(sItemString, strlen(DASHES) + 1, DASHES);
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = TAG_COLOR_GREY;
			return;
		}

		int seq;
		seq = m_SequenceArray[seqpos.m_index].m_status % 2 ? seqpos.m_sequence : 0;

		if (seq) { // stby
			sprintf_s(sItemString, strlen(DASHES) + 1, "%s-%.2d",
				STATUS_DESCRIPTION[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2], seq);
		}
		else { // clrd
			sprintf_s(sItemString, strlen(DASHES) + 1, "%s---",
				STATUS_DESCRIPTION[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2]);
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = TAG_COLOR_BLUE;
		}
	}
}

void CSequencingPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	EuroScopePlugIn::CFlightPlan fp;
	SequenceData seqnew;
	SequencePosition seqpos;

	fp = FlightPlanSelectASEL();
	if (!fp.IsValid())
		return;

	seqpos = GetSequenceData(fp.GetCallsign());

	if (seqpos.m_index == M_ARRAY_NOMATCH
		&& FunctionId != TAG_FUNC_SEQ_POPUP && FunctionId != TAG_FUNC_SEQ_START)
		return;

	switch (FunctionId)
	{

	case TAG_FUNC_SEQ_POPUP:

		OpenPopupList(Area, "SEQ Menu", 2);

		if (seqpos.m_index == M_ARRAY_NOMATCH)
			AddPopupListElement("Start", "SEQ", TAG_FUNC_SEQ_START);
		else {
			AddPopupListElement("Next", "STS", TAG_FUNC_SEQ_NEXT);
			AddPopupListElement("Prev", "STS", TAG_FUNC_SEQ_PREV);
			if (m_SequenceArray[seqpos.m_index].m_status % 2) // stby status
				AddPopupListElement("Edit", "SEQ", TAG_FUNC_SEQ_EDIT_POPUP);
			AddPopupListElement("Reset", "STS", TAG_FUNC_SEQ_RESET);
			AddPopupListElement("Delete", "", TAG_FUNC_SEQ_DELETE);
		}

		break;
	case TAG_FUNC_SEQ_START:
	{
		SequenceData seqstart = {
			(char*)fp.GetCallsign(),
			GroundStatus::STBY_CLEARANCE,
			true };
		m_SequenceArray.Add(seqstart);
		break;
	}
	case TAG_FUNC_SEQ_RESET:

		seqnew = m_SequenceArray[seqpos.m_index];
		m_SequenceArray.RemoveAt(seqpos.m_index);
		seqnew.m_status = GroundStatus::STBY_CLEARANCE;
		m_SequenceArray.Add(seqnew);
		break;

	case TAG_FUNC_SEQ_NEXT:

		seqnew = m_SequenceArray[seqpos.m_index];
		if (++seqnew.m_status <= GroundStatus::CLRD_TAKEOFF) {
			m_SequenceArray.RemoveAt(seqpos.m_index);
			m_SequenceArray.Add(seqnew);
		}
		break;

	case TAG_FUNC_SEQ_PREV:

		seqnew = m_SequenceArray[seqpos.m_index];
		if (--seqnew.m_status >= GroundStatus::STBY_CLEARANCE) {
			m_SequenceArray.RemoveAt(seqpos.m_index);
			m_SequenceArray.Add(seqnew);
		}
		break;

	case TAG_FUNC_SEQ_EDIT_POPUP:

		OpenPopupEdit(Area, TAG_FUNC_SEQ_EDIT_FINISH, "");
		break;

	case TAG_FUNC_SEQ_EDIT_FINISH:

		int ns; // new sequence
		if (sscanf_s(sItemString, "%d", &ns) > 0) {
			int idx, ary, seq[3] = { 0,0,0 }, tidx;

			for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) { // to locate where to insert
				ary = (m_SequenceArray[idx].m_status - 1) / 2; // array index
				if (m_SequenceArray[idx].m_active && m_SequenceArray[idx].m_status % 2) // stby status
					++seq[ary];
				if (seq[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2] == ns)
					tidx = idx;
			}

			if (ns <= seq[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2]
				&& ns > 0) { // valid edit: between 0 and max sequence of the status
				SequenceData seqnew = m_SequenceArray[seqpos.m_index];
				m_SequenceArray.RemoveAt(seqpos.m_index);
				m_SequenceArray.InsertAt(tidx, seqnew);
			}
		}
		break;

	case TAG_FUNC_SEQ_DELETE:

		m_SequenceArray.RemoveAt(seqpos.m_index);
		break;

	default:
		break;
	}
}

bool CSequencingPlugIn::OnCompileCommand(const char* sCommandLine) {
	if (strncmp(sCommandLine, ".dls", 4) || strlen(sCommandLine) < 6) // undefined
		return false;

	CString cmd = sCommandLine + 5;
	cmd.MakeUpper();

	if (cmd == "REMOVE ALL") {
		m_SequenceArray.RemoveAll();
		DisplayUserMessage("DLS PlugIn", "", "All sequence removed!",
			false, true, true, true, true);
		return true;
	}

	if (cmd == "REMOVE OFFLINE") {
		int idx;
		for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
			if (!IsCallsignOnline(m_SequenceArray[idx].m_callsign))
				m_SequenceArray.RemoveAt(idx);
		DisplayUserMessage("DLS PlugIn", "", "All offline sequence removed!",
			false, true, true, true, true);
		return true;
	}

	return false;
}

void CSequencingPlugIn::OnTimer(int Counter) {
	// update active information for all aircrafts
	if (Counter % 5) // once every 5 seconds
		return;

	CRadarTarget rt;
	int idx;
	bool online;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		online = IsCallsignOnline(m_SequenceArray[idx].m_callsign);
		m_SequenceArray[idx].m_active = online;
		if (!online) { // avoid potential errors
			TRACE("%s\tdeactivate\n", m_SequenceArray[idx].m_callsign);
			continue;
		}
		rt = RadarTargetSelect(m_SequenceArray[idx].m_callsign);
		if (rt.GetGS() > 60) { // criterion for taking off
			TRACE("%s\tremoved\n", m_SequenceArray[idx].m_callsign);
			m_SequenceArray.RemoveAt(idx);
		}
	}
}