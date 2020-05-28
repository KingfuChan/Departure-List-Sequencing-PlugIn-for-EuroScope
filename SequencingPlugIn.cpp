#include "pch.h"
#include "SequencingPlugIn.h"


#define PLUGIN_NAME "Departure List Sequencing"
#define PLUGIN_VERSION "0.9.4"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "Copyright (C) 2020, Kingfu Chan"


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
const char* SETTINGS_COLOR_INACTIVE = "ColorInac";
const char* SETTINGS_COLOR_CLEARED = "ColorClrd";

// refresh interval
const int DEFAULT_REFRESH_INTERVAL = 5;
const char* SETTINGS_INTERVAL = "RefreshIntv";

// maximum speed
const int DEFAULT_MAX_SPEED = 80; // maximum speed before delete from memory
const char* SETTINGS_MAX_SPEED = "MaxSpeed";

// tag texts
const char PLACE_HOLDER[] = "-------"; // place-holder
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
	RegisterTagItemFunction("GND SEQ Popup List", TAG_FUNC_SEQ_POPUP);

	m_SequenceArray.RemoveAll();

	// load settings
	CustomColorInac = TAG_COLOR_GREY;
	CustomColorClrd = TAG_COLOR_BLUE;
	ParseColorFromText(GetDataFromSettings(SETTINGS_COLOR_INACTIVE), CustomColorInac);
	ParseColorFromText(GetDataFromSettings(SETTINGS_COLOR_CLEARED), CustomColorClrd);

	const char* itvchar = GetDataFromSettings(SETTINGS_INTERVAL);
	CustomRefreshIntv = itvchar == nullptr ?
		DEFAULT_REFRESH_INTERVAL : atoi(itvchar);

	const char* spdchar = GetDataFromSettings(SETTINGS_MAX_SPEED);
	CustomMaxSpeed = spdchar == nullptr ?
		DEFAULT_MAX_SPEED : atoi(spdchar);

}


CSequencingPlugIn::~CSequencingPlugIn(void) {
}


// custom funcitons


SequencePosition CSequencingPlugIn::GetSequenceData(const char* callsign) {
	//also calculates sequence number, note that may not be able to change data
	int ary, idx, seq[3] = { 0,0,0 };
	SequencePosition sp;

	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		ary = (m_SequenceArray[idx].m_status - 1) / 2; // array index
		if (!m_SequenceArray[idx].m_active) // double check online, avoid potential bugs on refresh
			m_SequenceArray[idx].m_active = IsCallsignOnline(callsign);
		if (m_SequenceArray[idx].m_active && m_SequenceArray[idx].m_status % 2) // stby status
			++seq[ary];
		if (!strcmp(m_SequenceArray[idx].m_callsign, callsign)) {
			sp.m_index = idx;
			sp.m_sequence = seq[ary];
			return sp;
		}
	}

	sp.m_index = M_ARRAY_NOMATCH;
	sp.m_sequence = M_ARRAY_NOMATCH;
	return sp;
}


bool CSequencingPlugIn::IsCallsignOnline(const char* callsign) {
	CRadarTarget rt;
	for (rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt))
		if (!strcmp(callsign, rt.GetCallsign()))
			return true;
	return false;
}


bool CSequencingPlugIn::ParseColorFromText(const char* text, COLORREF& color) {
	if (text == nullptr)
		return false;

	int r, g, b;
	if (sscanf_s(text, "%d,%d,%d", &r, &g, &b) == 3)
		if (r >= 0 && r <= 255 &&
			g >= 0 && r <= 255 &&
			b >= 0 && b <= 255) { // valid color value
			color = RGB(r, g, b);
			return true;
		}
	return false;
}


void CSequencingPlugIn::DisplayMessage(const char* msg) {
	// unified display method with fixed parameters
	DisplayUserMessage("Message", "DLS Plugin",
		msg,
		false, true, true, false, false);
}


// below are inherited functions


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
			strcpy_s(sItemString, strlen(PLACE_HOLDER) + 1, PLACE_HOLDER);
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = CustomColorInac;
			return;
		}

		int seq;
		seq = m_SequenceArray[seqpos.m_index].m_status % 2 ? seqpos.m_sequence : 0;

		if (seq) { // stby
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s-%.2d",
				STATUS_DESCRIPTION[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2], seq);
		}
		else { // clrd
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s---",
				STATUS_DESCRIPTION[(m_SequenceArray[seqpos.m_index].m_status - 1) / 2]);
			*pColorCode = TAG_COLOR_RGB_DEFINED;
			*pRGB = CustomColorClrd;
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

			// little bit tricky :D
			int sts = m_SequenceArray[seqpos.m_index].m_status;
			if (sts < GroundStatus::CLRD_TAKEOFF)
				AddPopupListElement(STATUS_DESCRIPTION[sts / 2], sts % 2 ? "clrd" : "stby", TAG_FUNC_SEQ_NEXT);

			if (sts > GroundStatus::STBY_CLEARANCE)
				AddPopupListElement(STATUS_DESCRIPTION[sts / 2 - 1], sts % 2 ? "clrd" : "stby", TAG_FUNC_SEQ_PREV);

			if (sts % 2) // stby status
				AddPopupListElement("Edit", "SEQ", TAG_FUNC_SEQ_EDIT_POPUP);

			AddPopupListElement("Reset", "STS", TAG_FUNC_SEQ_RESET);
			AddPopupListElement("Delete", "STS", TAG_FUNC_SEQ_DELETE);
		}

		break;
	case TAG_FUNC_SEQ_START:
	{
		SequenceData seqstart = {
			fp.GetCallsign(),
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
		if (++seqnew.m_status <= GroundStatus::CLRD_TAKEOFF) { // shouldn't be true
			m_SequenceArray.RemoveAt(seqpos.m_index);
			m_SequenceArray.Add(seqnew);
		}
		break;

	case TAG_FUNC_SEQ_PREV:

		seqnew = m_SequenceArray[seqpos.m_index];
		if (--seqnew.m_status >= GroundStatus::STBY_CLEARANCE) { // shouldn't be true
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
	CString cmd = sCommandLine;
	cmd.Trim();
	cmd.MakeUpper();
	if (cmd.Left(4) == ".DLS " || cmd.GetLength() < 5) // undefined
		return false;

	cmd = cmd.Mid(5);
	CString msg; // for display

	if (cmd == "REMOVE ALL") {
		m_SequenceArray.RemoveAll();
		DisplayMessage("All sequences have been removed!");
		return true;
	}

	if (cmd == "REMOVE OFFLINE") {
		int idx;
		for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
			if (!IsCallsignOnline(m_SequenceArray[idx].m_callsign))
				m_SequenceArray.RemoveAt(idx);
		DisplayMessage("All offline sequences have been removed!");
		return true;
	}

	// color format: rrr,ggg,bbb (no space, English comma)
	if (cmd.Left(15) == "COLOR INACTIVE ") { // 15==strlen
		const char* colordata = (LPCTSTR)cmd + 15;
		bool success = ParseColorFromText(colordata, CustomColorInac);
		if (success) {
			SaveDataToSettings(SETTINGS_COLOR_INACTIVE, "custom color for inactive tags", colordata);
			msg.Format("Custom color for inactive tags have been set to (%s)!", colordata);
			DisplayMessage(msg);
		}
		return success;
	}

	if (cmd.Left(14) == "COLOR CLEARED ") { // 14==strlen
		const char* colordata = (LPCTSTR)cmd + 14;
		bool success = ParseColorFromText(colordata, CustomColorClrd);
		if (success) {
			SaveDataToSettings(SETTINGS_COLOR_CLEARED, "custom color for cleared tags", colordata);
			msg.Format("Custom color for cleared tags have been set to (%s)!", colordata);
			DisplayMessage(msg);
		}
		return success;
	}

	if (cmd == "COLOR RESET") {
		CustomColorClrd = TAG_COLOR_BLUE;
		CustomColorInac = TAG_COLOR_GREY;
		CString saveinac, saveclrd;
		saveinac.Format("%d,%d,%d",
			GetRValue(TAG_COLOR_GREY), GetGValue(TAG_COLOR_GREY), GetBValue(TAG_COLOR_GREY));
		saveclrd.Format("%d,%d,%d",
			GetRValue(TAG_COLOR_BLUE), GetGValue(TAG_COLOR_BLUE), GetBValue(TAG_COLOR_BLUE));
		SaveDataToSettings(SETTINGS_COLOR_INACTIVE, "reset color for inactive tags", saveinac);
		SaveDataToSettings(SETTINGS_COLOR_CLEARED, "reset color for cleared tags", saveclrd);
		DisplayMessage("Default colors for tags have been set!");
		return true;
	}

	if (cmd.Left(9) == "INTERVAL ") { // 9==strlen
		CString itvstr = cmd.Mid(9);
		int itv = atoi(itvstr);
		if (itv >= 1) {
			CustomRefreshIntv = itv;
			SaveDataToSettings(SETTINGS_INTERVAL, "custom refresh interval", itvstr);
			msg.Format("Refresh interval has been set to %d seconds!", itv);
			DisplayMessage(msg);
			return true;
		}
	}

	if (cmd == "INTERVAL RESET") {
		CustomRefreshIntv = DEFAULT_REFRESH_INTERVAL;
		CString itvstr;
		itvstr.Format("%d", CustomRefreshIntv);
		SaveDataToSettings(SETTINGS_INTERVAL, "reset refresh interval", itvstr);
		DisplayMessage("Default refresh interval has been set!");
		return true;
	}

	if (cmd.Left(6) == "SPEED ") { // 6==strlen
		CString spdstr = cmd.Mid(6);
		int spd = atoi(spdstr);
		if (spd > 30) {
			CustomMaxSpeed = spd;
			SaveDataToSettings(SETTINGS_MAX_SPEED, "custom maximum speed", spdstr);
			msg.Format("Maximum speed has been set to %d knots!", spd);
			DisplayMessage(msg);
			return true;
		}
	}

	if (cmd == "SPEED RESET") {
		CustomMaxSpeed = DEFAULT_MAX_SPEED;
		CString spdstr;
		spdstr.Format("%d", CustomMaxSpeed);
		SaveDataToSettings(SETTINGS_MAX_SPEED, "reset maximum speed", spdstr);
		DisplayMessage("Default maximum speed has been set!");
		return true;
	}

	return false;
}


void CSequencingPlugIn::OnTimer(int Counter) {
	// update active information for all aircrafts
	if (Counter % CustomRefreshIntv) // once every n seconds
		return;

	CRadarTarget rt;
	int idx;
	bool online;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		online = IsCallsignOnline(m_SequenceArray[idx].m_callsign);
		m_SequenceArray[idx].m_active = online;
		if (!online) { // avoid potential errors
			TRACE("%s\tinactive\n", m_SequenceArray[idx].m_callsign);
			continue;
		}
		rt = RadarTargetSelect(m_SequenceArray[idx].m_callsign);
		if (rt.GetGS() > CustomMaxSpeed) { // criterion for taking off
			TRACE("%s\tremoved\n", m_SequenceArray[idx].m_callsign);
			m_SequenceArray.RemoveAt(idx);
		}
	}
}
