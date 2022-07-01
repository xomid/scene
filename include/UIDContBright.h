#pragma once
#include "UIDEffect.h"
#include "UIComplexS.h"
#include <thread>
#include <chrono>

class UIDContBright : public UIDEffect
{
	UIComplexS scBright, scContst;
	UICheck chkLegacy;
	bool bLegacy;

	/*int bright, lbright, contrast, lcontrast;
	bool bLegacy;
	pyte blookup, clookup;


	void on_init() override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;

	virtual void PopUp();
	virtual void Apply();
	virtual void StopThread();

	static DWORD WINAPI ApplyEffect(LPVOID lParam);
	static DWORD WINAPI ApplyCompleteEffect(LPVOID lParam);
	afx_msg void OnBtnLegacy();*/

public:
	void measure_size(int* width, int* height) override;
	void on_init() override;
	void on_resize(int width, int height) override;
	void process_event(OUI* element, uint32_t message, uint64_t param, bool bubbleUp) override;
	void apply() override;
	void render() override;
};
