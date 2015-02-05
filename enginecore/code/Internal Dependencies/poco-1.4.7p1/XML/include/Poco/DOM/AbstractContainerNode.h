//
// AbstractContainerNode.h
//
// $Id: //poco/1.4/XML/include/Poco/DOM/AbstractContainerNode.h#2 $
//
// Library: XML
// Package: DOM
// Module:  DOM
//
// Definition of the AbstractContainerNode class.
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


#ifndef DOM_AbstractContainerNode_INCLUDED
#define DOM_AbstractContainerNode_INCLUDED


#include "Poco/XML/XML.h"
#include "Poco/DOM/AbstractNode.h"


namespace Poco {
namespace XML {


class XML_API AbstractContainerNode: public AbstractNode
	/// AbstractContainerNode is an implementation of Node
	/// that stores and manages child nodes.
	///
	/// Child nodes are organized in a single linked list.
{
public:
	// Node
	Node* firstChild() const;
	Node* lastChild() const;
	Node* insertBefore(Node* newChild, Node* refChild);
	Node* replaceChild(Node* newChild, Node* oldChild);
	Node* removeChild(Node* oldChild);
	Node* appendChild(Node* newChild);
	bool hasChildNodes() const;
	bool hasAttributes() const;
	Node* getNodeByPath(const XMLString& path) const;
	Node* getNodeByPathNS(const XMLString& path, const NSMap& nsMap) const;

protected:
	AbstractContainerNode(Document* pOwnerDocument);
	AbstractContainerNode(Document* pOwnerDocument, const AbstractContainerNode& node);
	~AbstractContainerNode();

	void dispatchNodeRemovedFromDocument();
	void dispatchNodeInsertedIntoDocument();
	
	static const Node* findNode(XMLString::const_iterator& it, const XMLString::const_iterator& end, const Node* pNode, const NSMap* pNSMap);
	static const Node* findElement(const XMLString& name, const Node* pNode, const NSMap* pNSMap);
	static const Node* findElement(int index, const Node* pNode, const NSMap* pNSMap);
	static const Node* findElement(const XMLString& attr, const XMLString& value, const Node* pNode, const NSMap* pNSMap);
	static const Attr* findAttribute(const XMLString& name, const Node* pNode, const NSMap* pNSMap);
	bool hasAttributeValue(const XMLString& name, const XMLString& value, const NSMap* pNSMap) const;
	static bool namesAreEqual(const Node* pNode1, const Node* pNode2, const NSMap* pNSMap);
	static bool namesAreEqual(const Node* pNode, const XMLString& name, const NSMap* pNSMap);

private:
	AbstractNode* _pFirstChild;

	friend class AbstractNode;
	friend class NodeAppender;
};


} } // namespace Poco::XML


#endif // DOM_AbstractContainerNode_INCLUDED
