#ifndef ANTLR3CXX_PARSER_HPP_
#define ANTLR3CXX_PARSER_HPP_

#ifndef LEXER_INCLUDE
#error "Please define LEXER_INCLUDE"
#endif

#ifndef PARSER_INCLUDE
#error "Please define PARSER_INCLUDE"
#endif

extern "C" {
#include "antlr3.h"
#include LEXER_INCLUDE
#include PARSER_INCLUDE
}

namespace antlr { 
	typedef ANTLR3_TOKEN_SOURCE TokenSource;
    typedef ANTLR3_COMMON_TOKEN CommonToken;
    typedef ANTLR3_INPUT_STREAM ANTLRInputStream;
    typedef ANTLR3_COMMON_TOKEN_STREAM CommonTokenStream;
	typedef ANTLR3_STRING_FACTORY StringFactory;
	typedef ANTLR3_ARBORETUM Arboretum;
namespace tree { 
	typedef ANTLR3_BASE_TREE Tree; 
    typedef ANTLR3_COMMON_TREE CommonTree;
} }

#include <sstream>
#include <vector>
#include <string>
#include <cassert>
using std::vector;

#define GET_TEXT(node) (node)->getText((node))
#define TO_STRING(node) (node)->toString((node))
#define GET_TYPE(node) (node)->getType((node))
#define GET_PARENT(node) (node)->getParent((node))
#define GET_CHILD_COUNT(node) (node)->getChildCount((node))
#define TO_STRING_TREE(node) (node)->toStringTree((node))

// HACK: libantlr3c doesn't seem to create parent pointers right now. So when we
// get a child, update its parent pointer.
static inline antlr::tree::Tree *get_child_(antlr::tree::Tree *n, int i)
{
	antlr::tree::Tree *child = reinterpret_cast<antlr::tree::Tree *>(n->getChild(n, i));
	if (child) ((pANTLR3_COMMON_TREE)(child->super))->parent = (pANTLR3_COMMON_TREE)(n->super);
	return child;
}

#define GET_CHILD(node, i) (get_child_((node), (i)))
#define TOKEN(tokname) tokname
#define GET_FACTORY(node) (((pANTLR3_COMMON_TREE) (node)->super)->factory)

/* Since antlr doesn't provide us with named tree elements, or a convenient way of
 * querying for subtrees (except using tree grammars), let's define some nasty
 * macros which avoid much of the pain. */

/* Evaluate to true, but with a side-effect of performing an assignment. */
#define ASSIGN_AS_COND(name, value) \
	(((name) = (value)) == (name))

/* For-loop over all children of a node, binding some useful variable names
 * globally: childcount
 * per-iteration: i (child index), n (child tree), text (text) */	
#define FOR_ALL_CHILDREN(t) unsigned i = 0; \
	FOR_BODY(t)	

#define FOR_REMAINING_CHILDREN(t) unsigned i = next_child_to_bind; \
	FOR_BODY(t)
	
#define FOR_BODY(t) \
	antlr::tree::Tree *__tree_head_pointer = reinterpret_cast<antlr::tree::Tree *>(t); /* because our tree may well alias 'n' */ \
	unsigned childcount; \
	const char *text __attribute__((unused)) = 0; \
	antlr::tree::Tree *n = 0; \
	for (childcount = GET_CHILD_COUNT(__tree_head_pointer), \
		n = ((childcount > 0) ? reinterpret_cast<antlr::tree::Tree*>(GET_CHILD(__tree_head_pointer, 0)) : 0), \
		text = (n != 0 && ((GET_TEXT(n)) != 0)) ? CCP(GET_TEXT(n)) : "(null)"; \
	i < childcount && (n = reinterpret_cast<antlr::tree::Tree*>(GET_CHILD(__tree_head_pointer, i)), true) && \
		(( text = ((n != 0 && ((GET_TEXT(n)) != 0)) ? CCP(GET_TEXT(n)) : "(null)") ), true); \
	i++)

#define CHECK_TOKEN(node, token, tokenname) \
	if (!(GET_TYPE(node) == token)) throw antlr3cxx::SemanticError((node), \
	(dynamic_cast<std::ostringstream&>((std::ostringstream() << "expected a token of class " << TOKEN(token) << "/" << tokenname \
	" (" __FILE__ ":" << __LINE__ << "); found token " << CCP(GET_TEXT(node)) \
	<< " class id " << GET_TYPE(node))).str() )) 

/* Before binding a sequence of children, do INIT. 
 * Don't do INIT more than once in the same scope -- start another scope instead. */
#define INIT int next_child_to_bind __attribute__(( unused )) = 0 
#define BIND2(node, name) antlr::tree::Tree *name __attribute__((unused)) = reinterpret_cast<antlr::tree::Tree*>(GET_CHILD(node, next_child_to_bind++));
#define BIND3(node, name, token) antlr::tree::Tree *name __attribute__((unused)) = reinterpret_cast<antlr::tree::Tree*>(GET_CHILD(node, next_child_to_bind++)); \
	if ((name) == 0) throw antlr3cxx::SemanticError( \
		(name), \
		"no child node!"); \
	CHECK_TOKEN(name, token, #token) \
	//std::cerr << "DEBUG: " __FILE__ ":" << __LINE__ << " bound a token of type " << (int) ((name)->getType()) << "(" #token ") to name " #name << ", text " << CCP((name)->getText()) << std::endl

/* Skip over tokens we're not interested in. */
#define SELECT_NOT(token) if (GET_TYPE(n) == (token)) continue
#define SELECT_ONLY(token) if (GET_TYPE(n) != (token)) continue

/* Make a C-style string out of a Java one. */
#define CCP(p) ((p) ? reinterpret_cast<char*>((p->chars)) : "(no text)")

/* Throw a semantic error for token n */
#define SEMANTIC_ERROR(n) throw antlr3cxx::SemanticError( \
							(n), std::string("Malformed AST: found an unexpected token: ")+std::string(CCP(GET_TEXT(n))))

#define ALIAS2(node, name) antlr::tree::Tree *& name = (node)
#define ALIAS3(node, name, token) \
	antlr::tree::Tree *& name = (node); \
	CHECK_TOKEN(name, token, #token);

#define RAISE(node, msg) throw antlr3cxx::SemanticError((node), std::string(msg) \
					+ ": " \
						+ std::string(CCP(TO_STRING_TREE(node))))

#define RAISE_INTERNAL(node, msg) throw antlr3cxx::InternalError((node), std::string( \
					msg ": ") \
						+std::string(CCP(TO_STRING_TREE(node))))

namespace antlr3cxx {

	using std::string; 
	inline antlr::tree::Tree *clone_ast(antlr::tree::Tree *t)
	{
		#define CLONE(t)  GET_FACTORY(t)->newFromTree(GET_FACTORY(t), (pANTLR3_COMMON_TREE)t->super)
		antlr::tree::Tree *cloned_head = CLONE(t);
		for (unsigned i = 0; i < GET_CHILD_COUNT(t); ++i)
		{
			cloned_head->addChild(cloned_head, clone_ast(GET_CHILD(t, i)));
		}
		return cloned_head;
		#undef CLONE
	}

	inline antlr::tree::Tree *build_ast(
		antlr::Arboretum *treeFactory,
		int tokenType, const string& text, 
		const std::vector<antlr::tree::Tree *>& children
	)
	{
		auto p_token = antlr3CommonTokenNew(tokenType);
		auto p_node = treeFactory->newFromToken(treeFactory, p_token);
		assert(p_node->strFactory);
		p_token->setText(p_token, p_node->strFactory->newPtr8(
				p_node->strFactory,
				(pANTLR3_UINT8) text.c_str(),
				text.length()
			)
		);
		
		// add the children
		for (auto i_child = children.begin(); i_child != children.end(); ++i_child)
		{
			p_node->addChild(p_node, *i_child);
		}
		
		return p_node;
	}

	class TreewalkError
	{
		antlr::tree::Tree *m_t;
		std::string m_msg;
	public:
		TreewalkError(antlr::tree::Tree *t, std::string msg) : m_t(t), m_msg(msg) {}
		std::string message() {
			std::ostringstream s;
			s << "TreewalkError";
			if (m_t != 0) 
			{
				antlr::CommonToken *tok = m_t->getToken(m_t);
				s << " at input position "
					<< tok->getLine(tok)
					<< ":" 
					<< tok->getCharPositionInLine(tok);
			}
			s << ": " << m_msg;
			return s.str();
		}
	};
	class SemanticError
	{
		antlr::tree::Tree *m_t;
		std::string m_msg;
	public:
		SemanticError(antlr::tree::Tree *t, std::string msg) : m_t(t), m_msg(msg) {}
		SemanticError(std::string msg) : m_t(0), m_msg(msg) {}
		std::string message() {
			std::ostringstream s;
			s << "SemanticError";
			if (m_t != 0) 
			{
				antlr::CommonToken *tok = m_t->getToken(m_t);
				s << " at input position "
					<< tok->getLine(tok)
					<< ":" 
					<< tok->getCharPositionInLine(tok);
			}
			s << ": " << m_msg;
			return s.str();
		}
	};
	class InternalError
	{
		antlr::tree::Tree *m_t;
		std::string m_msg;
	public:
		InternalError(antlr::tree::Tree *t, std::string msg) : m_t(t), m_msg(msg) {}
		std::string message() {
			std::ostringstream s;
			s << "InternalError";
			if (m_t != 0) 
			{
				antlr::CommonToken *tok = m_t->getToken(m_t);
				s << " at input position "
					<< tok->getLine(tok)
					<< ":" 
					<< tok->getCharPositionInLine(tok);
			}
			s << ": " << m_msg;
			return s.str();
		}
	};
	template <typename Func>
	void walk_ast_depthfirst(
		antlr::tree::Tree *t, 
		std::vector<antlr::tree::Tree *>& out,
		const Func& is_interesting)
	{
		if (t)
		{
			if (is_interesting(t)) out.push_back(t);
			for (unsigned i = 0U; i < GET_CHILD_COUNT(t); ++i)
			{
				walk_ast_depthfirst(GET_CHILD(t, i), out, is_interesting);
			}
		}
	}	
}

#endif
