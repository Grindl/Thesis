//
// WinRegistryTest.cpp
//
// $Id: //poco/1.4/Util/testsuite/src/WinRegistryTest.cpp#2 $
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#if !defined(_WIN32_WCE)


#include "WinRegistryTest.h"
#include "CppUnit/TestCaller.h"
#include "CppUnit/TestSuite.h"
#include "Poco/Util/WinRegistryKey.h"
#include "Poco/Environment.h"
#include "Poco/Exception.h"


using Poco::Util::WinRegistryKey;
using Poco::Environment;


WinRegistryTest::WinRegistryTest(const std::string& name): CppUnit::TestCase(name)
{
}


WinRegistryTest::~WinRegistryTest()
{
}


void WinRegistryTest::testRegistry()
{
	WinRegistryKey regKey("HKEY_CURRENT_USER\\Software\\Applied Informatics\\Test");
	if (regKey.exists())
	{
		regKey.deleteKey();
	}
	assert (!regKey.exists());
	regKey.setString("name1", "value1");
	assert (regKey.getString("name1") == "value1");
	regKey.setString("name1", "Value1");
	assert (regKey.getString("name1") == "Value1");
	regKey.setString("name2", "value2");
	assert (regKey.getString("name2") == "value2");
	assert (regKey.exists("name1"));
	assert (regKey.exists("name2"));
	assert (regKey.exists());
	
	WinRegistryKey regKeyRO("HKEY_CURRENT_USER\\Software\\Applied Informatics\\Test", true);
	assert (regKeyRO.getString("name1") == "Value1");
	try
	{
		regKeyRO.setString("name1", "newValue1");
	}
	catch (Poco::Exception& exc)
	{
		std::string msg = exc.displayText();
	}
	assert (regKey.getString("name1") == "Value1");
	
	WinRegistryKey::Values vals;
	regKey.values(vals);
	assert (vals.size() == 2);
	assert (vals[0] == "name1" || vals[0] == "name2");
	assert (vals[1] == "name1" || vals[1] == "name2");
	assert (vals[0] != vals[1]);

	Environment::set("FOO", "bar");
	regKey.setStringExpand("name3", "%FOO%");
	assert (regKey.getStringExpand("name3") == "bar");
	
	regKey.setInt("name4", 42);
	assert (regKey.getInt("name4") == 42);
	
	assert (regKey.exists("name4"));
	regKey.deleteValue("name4");
	assert (!regKey.exists("name4"));
	
	regKey.deleteKey();
	assert (!regKey.exists());
}


void WinRegistryTest::setUp()
{
}


void WinRegistryTest::tearDown()
{
}


CppUnit::Test* WinRegistryTest::suite()
{
	CppUnit::TestSuite* pSuite = new CppUnit::TestSuite("WinRegistryTest");

	CppUnit_addTest(pSuite, WinRegistryTest, testRegistry);

	return pSuite;
}


#endif // !defined(_WIN32_WCE)
