#pragma once

#pragma pack(push, 1)

// Commax

#define COMMAX_FIRE_ALARM_FIRE_OCCURED	1
#define COMMAX_FIRE_ALARM_FIRE_CLEAR	2
#define COMMAX_FIRE_ALARM_ALL_CLEAR		3

enum {
	XML_TAG_START_COMMAX,
	XML_TAG_END_COMMAX,
	XML_TAG_START_FIRE,
	XML_TAG_END_FIRE,
	XML_TAG_START_EMERGENCY,
	XML_TAG_END_EMERGENCY,
	XML_TAG_START_DONG,
	XML_TAG_END_DONG,
	XML_TAG_START_FLOOR,
	XML_TAG_END_FLOOR,
	XML_TAG_START_STAIR,
	XML_TAG_END_STAIR,
	XML_TAG_START_LINE,
	XML_TAG_END_LINE,
	XML_TAG_START_RESPONSE,
	XML_TAG_END_RESPONSE
} COMMAX_XML_TAG_ENUM;

//프로세스 명
static const TCHAR* g_lpszCommaxTagName[] = {
	_T("<cmx>"),
	_T("</cmx>"),
	_T("<fire>"),
	_T("</fire>"),
	_T("<emergency>"),
	_T("</emergency>"),
	_T("<dong>"),
	_T("</dong>"),
	_T("<floor>"),
	_T("</floor>"),
	_T("<stair>"),
	_T("</stair>"),
	_T("<line>"),
	_T("</line>"),
	_T("<response>"),
	_T("</response>")
};