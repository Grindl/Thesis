//
// PropertyFileConfiguration.cpp
//
// $Id: //poco/1.4/Util/src/PropertyFileConfiguration.cpp#2 $
//
// Library: Util
// Package: Configuration
// Module:  PropertyFileConfiguration
//
// Copyright (c) 2004-2009, Applied Informatics Software Engineering GmbH.
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


#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/Exception.h"
#include "Poco/String.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/LineEndingConverter.h"
#include "Poco/Ascii.h"


using Poco::trim;
using Poco::Path;


namespace Poco {
namespace Util {


PropertyFileConfiguration::PropertyFileConfiguration()
{
}


PropertyFileConfiguration::PropertyFileConfiguration(std::istream& istr)
{
	load(istr);
}

	
PropertyFileConfiguration::PropertyFileConfiguration(const std::string& path)
{
	load(path);
}


PropertyFileConfiguration::~PropertyFileConfiguration()
{
}

	
void PropertyFileConfiguration::load(std::istream& istr)
{
	clear();
	while (!istr.eof())
	{
		parseLine(istr);
	}
}

	
void PropertyFileConfiguration::load(const std::string& path)
{
	Poco::FileInputStream istr(path);
	if (istr.good())
		load(istr);
	else
		throw Poco::OpenFileException(path);
}


void PropertyFileConfiguration::save(std::ostream& ostr) const
{
	MapConfiguration::iterator it = begin();
	MapConfiguration::iterator ed = end();
	while (it != ed)
	{
		ostr << it->first << ": ";
		for (std::string::const_iterator its = it->second.begin(); its != it->second.end(); ++its)
		{
			switch (*its)
			{
			case '\t':
				ostr << "\\t";
				break;
			case '\r':
				ostr << "\\r";
				break;
			case '\n':
				ostr << "\\n";
				break;
			case '\f':
				ostr << "\\f";
				break;
			case '\\':
				ostr << "\\\\";
				break;
			default:
				ostr << *its;
				break;
			}
		}
		ostr << "\n";
		++it;
	}
}


void PropertyFileConfiguration::save(const std::string& path) const
{
	Poco::FileOutputStream ostr(path);
	if (ostr.good())
	{
		Poco::OutputLineEndingConverter lec(ostr);
		save(lec);
		lec.flush();
		ostr.flush();
		if (!ostr.good()) throw Poco::WriteFileException(path);
	}
	else throw Poco::CreateFileException(path);
}


void PropertyFileConfiguration::parseLine(std::istream& istr)
{
	static const int eof = std::char_traits<char>::eof(); 

	int c = istr.get();
	while (c != eof && Poco::Ascii::isSpace(c)) c = istr.get();
	if (c != eof)
	{
		if (c == '#' || c == '!')
		{
			while (c != eof && c != '\n' && c != '\r') c = istr.get();
		}
		else
		{
			std::string key;
			while (c != eof && c != '=' && c != ':' && c != '\r' && c != '\n') { key += (char) c; c = istr.get(); }
			std::string value;
			if (c == '=' || c == ':')
			{
				c = readChar(istr);
				while (c != eof && c) { value += (char) c; c = readChar(istr); }
			}
			setRaw(trim(key), trim(value));
		}
	}
}


int PropertyFileConfiguration::readChar(std::istream& istr)
{
	for (;;)
	{
		int c = istr.get();
		if (c == '\\')
		{
			c = istr.get();
			switch (c)
			{
			case 't':
				return '\t';
			case 'r':
				return '\r';
			case 'n':
				return '\n';
			case 'f':
				return '\f';
			case '\r':
				if (istr.peek() == '\n')
					istr.get();
				continue;
			case '\n':
				continue;
			default:
				return c;
			}
		}
		else if (c == '\n' || c == '\r')
			return 0;
		else
			return c;
	}
}


} } // namespace Poco::Util
