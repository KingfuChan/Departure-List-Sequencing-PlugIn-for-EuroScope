#pragma once

#include <string>
#include <EuroScopePlugIn.h>

struct PluginCfg {
	int interval;
	int speed;
	CString sync;
};

struct SequenceData {
	CString m_callsign;
	int m_status;
	int m_syncstatus;
	bool m_active;
	int m_sequence;
};

using namespace EuroScopePlugIn;

class SeqDataPlugIn : public CPlugIn
{
public:
	SeqDataPlugIn(void);
	~SeqDataPlugIn(void);
	virtual void OnTimer(int Counter);
	void ConfigurePlugin(const int* interval, const int* speed, const char* sync);
	SequenceData GetSequence(CFlightPlan FlightPlan);
	void AddSequence(CFlightPlan FlightPlan);
	SequenceData PopSequence(CFlightPlan FlightPlan);
	void EditStatus(CFlightPlan FlightPlan, const int method);
	void EditSequence(const SequenceData seqdata, const int target_seq);
	void RemoveAll(void);
	void RemoveOffline(void);

private:
	CArray<SequenceData, SequenceData&> m_SequenceArray;
	PluginCfg activeConfig;
	int GetSyncStatus(CFlightPlan FlightPlan);
	void PutSyncStatus(CFlightPlan FlightPlan, int status);
	bool IsCallsignOnline(const char* callsign);
};
