#ifndef __DYNLAYOUT_PROFILE_HPP_INCLUDED__
#define __DYNLAYOUT_PROFILE_HPP_INCLUDED__

#pragma once

#ifdef LAYOUT_DLL_BUILD
	#define LAYOUT_API __declspec(dllexport)
#else
	#define LAYOUT_API __declspec(dllimport)
#endif

#include <memory>

namespace Layout
{
	class I_AppRegistryAdapter
	{
	public:
		virtual bool readLong(char const * pchApp, char const * pchSubpath, __out long & lResult, long lDefault) = 0;
		virtual bool writeLong(char const * pchApp, char const * pchSubpath, long lValue) = 0;
		virtual bool readString(char const * pchApp, char const * pchSubpath, __out char * pchValue, int iMaxLen, const char* pchDefault) = 0;
		virtual bool writeString(char const * pchApp, char const * pchSubpath, char const * pchValue) = 0;
	};

	class LAYOUT_API Registry
	{
	public:
		static Registry* getInstance() {if(_instance) return _instance; return _instance = new Registry;}
		void setAdapter(std::auto_ptr<I_AppRegistryAdapter> pAdapter) {_adapter = pAdapter;}
		bool readLong(char const * pchApp, char const * pchSubpath, __out long & lResult, long lDefault) {
			if(_adapter.get() != NULL) return _adapter->readLong(pchApp, pchSubpath, lResult, lDefault);
			else return (lResult = lDefault, false);
		}
		bool writeLong(char const * pchApp, char const * pchSubpath, long lValue) {
			if(_adapter.get() != NULL) return _adapter->writeLong(pchApp, pchSubpath, lValue);
			else return false;
		}
		bool readString(char const * pchApp, char const * pchSubpath, __out char* pchResult, int iMaxLen, const char* pchDefault = NULL) {
			if(_adapter.get() != NULL) return _adapter->readString(pchApp, pchSubpath, pchResult, iMaxLen, pchDefault);
			else return (pchDefault && iMaxLen >= strlen(pchDefault) ? (strcpy(pchResult, pchDefault), false) : false, false);
		}
		bool writeString(char const * pchApp, char const * pchSubpath, char const* pchValue) {
			if(_adapter.get() != NULL) return _adapter->writeString(pchApp, pchSubpath, pchValue);
			else return false;
		}
		
	private:
		static Registry* _instance;
		std::auto_ptr<I_AppRegistryAdapter> _adapter;
	};
}

#endif