#pragma once

#include "sctp_state.h"
#include "sctp.h"

class LightOff : public SctpState
{
public:
	void enter(Sctp* sctp) {}
	void toggle(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp);
	static SctpState& getInstance();

private:
	LightOff() {}
	LightOff(const LightOff& other);
	LightOff& operator=(const LightOff& other);
};

class LowIntensity : public SctpState
{
public:
	void enter(Sctp* sctp) {}
	void toggle(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp);
	static SctpState& getInstance();

private:
	LowIntensity() {}
	LowIntensity(const LowIntensity& other);
	LowIntensity& operator=(const LowIntensity& other);
};

class MediumIntensity : public SctpState
{
public:
	void enter(Sctp* sctp) {}
	void toggle(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp);
	static SctpState& getInstance();

private:
	MediumIntensity() {}
	MediumIntensity(const MediumIntensity& other);
	MediumIntensity& operator=(const MediumIntensity& other);
};

class HighIntensity : public SctpState
{
public:
	void enter(Sctp* sctp) {}
	void toggle(Sctp* sctp);
	void exit(Sctp* sctp) {}
	int id(Sctp* sctp);
	static SctpState& getInstance();

private:
	HighIntensity() {}
	HighIntensity(const HighIntensity& other);
	HighIntensity& operator=(const HighIntensity& other);
};