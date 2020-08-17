#pragma once

#include <string>
#include <EuroScopePlugIn.h>


struct SequenceData {
	CString m_callsign;
	int m_status;
	bool m_active;
	int m_sequence;
};


class SeqDataPlugIn :
	public EuroScopePlugIn::CPlugIn
{
public:
	SeqDataPlugIn(void);
	~SeqDataPlugIn(void);
	virtual void OnTimer(int Counter);
	void ConfigurePlugin(int* interval, int* speed);
	SequenceData GetSequence(const char* callsign);
	void AddSequence(const char* callsign);
	SequenceData PopSequence(const char* callsign);
	void EditStatus(const char* callsign, const int method);
	void EditSequence(const SequenceData seqdata, const int target_seq);
	void RemoveAll(void);
	void RemoveOffline(void);

private:
	CArray<SequenceData, SequenceData&> m_SequenceArray;
	int CustomRefreshIntv, CustomMaxSpeed;
	bool IsCallsignOnline(const char* callsign);
};
