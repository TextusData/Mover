/* Regexp.h -*- c++ -*-
 * Copyright (c) 2014 Ross Biro
 */

/*
 *   This program is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation, version 3 of the
 *   License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see
 *   <http://www.gnu.org/licenses/>.
 *
 */


#ifndef TEXTUS_UTIL_REGEXP_H_
#define TEXTUS_UTIL_REGEXP_H_

#include <string>
#include <list>
#include <map>
#include <set>

#include "textus/base/Base.h"
#include "textus/base/ReferenceList.h"
#include "textus/base/ReferenceValueMap.h"
#include "textus/util/Filters.h"
#include "textus/util/STLExtensions.h"

#define BEGINNING_OF_STRING_INDEX 0
#define END_OF_STRING_INDEX 1

namespace textus { namespace util {

using namespace textus::base;
using namespace std;


class Match: virtual public Base {
public:
  typedef pair<off_t, off_t> group_type;
  off_t begin;
  off_t end;
  size_t ngroups;
  list<int> group_stack;
  list<group_type> *group_matches;

public:
  explicit Match(int group_count) {
    ngroups = group_count;
    if (group_count) {
      group_matches = new list<pair<off_t, off_t> >[group_count];
    } else {
      group_matches = NULL;
    }
  }

  virtual ~Match() {
    if (group_matches) {
      delete [] group_matches;
    }
  }
};

template <typename C> class RegExp: virtual public Base {
private:
  class Node;

  class CompileNode: virtual public Base {
  private:
    friend class RegExp;
    friend class Node;

    ReferenceValueMap<int, CompileNode *> transitions;
    list<off_t> begin_group_list;
    list<off_t> end_group_list;
    list<int> bias_group_list;
    ReferenceList<CompileNode *> empty_transitions;
    CompileNode *replaced_by;
    int ngroup_begin; // number of groups that start here.
    int ngroup_end;  // number of groups that end here.
    bool accepting;
    Node *node;
    CompileNode *clone;

  public:
    explicit CompileNode(): replaced_by(NULL), ngroup_begin(0),
			    ngroup_end(0), accepting(false), node(NULL),
			    clone(NULL) {}
    virtual ~CompileNode() {}
  };
  AutoDeref<CompileNode> compile_nodes;


  class Node {
  private:
    friend class RegExp;
    friend class CompileNode;

    Node **transitions;
    bool accepting;
    int ngroup_begin; // number of groups that start here.
    int ngroup_end;  // number of groups that end here.
    off_t *group_begin;
    int *group_bias;
    off_t *group_end;
  public:
    explicit Node() {}
    virtual ~Node() {}
    int  fromCompileNode(CompileNode *cn) {
      int ret = 0;
      int count = 0;
      cn->node = this;
      accepting = cn->accepting;
      ngroup_begin = cn->ngroup_begin;
      ngroup_end = cn->ngroup_end;

      if (ngroup_begin) {
	group_begin = static_cast<off_t *>(malloc(sizeof(*group_begin)
						  * ngroup_begin));
	HRNULL(group_begin);
      } else {
	group_begin = NULL;
      }

      if (ngroup_end) {
	group_end = static_cast<off_t *>(malloc(sizeof(*group_end)
						* ngroup_end));
	HRNULL(group_end);
	group_bias = static_cast<int *>(malloc(sizeof(*group_bias)
					       * ngroup_end));
	HRNULL(group_bias);
      } else {
	group_end = NULL;
      }

      for (list<off_t>::iterator i = cn->begin_group_list.begin();
	   i != cn->begin_group_list.end(); ++i) {
	HRTRUE(count < ngroup_begin);
	group_begin[count++] = *i;
      }

      count = 0;
      for (list<int>::iterator i = cn->bias_group_list.begin();
	   i != cn->bias_group_list.end(); ++i) {
	HRTRUE(count < ngroup_end);
	group_bias[count++] = *i;
      }

      count = 0;
      for (list<off_t>::iterator i = cn->end_group_list.begin();
	   i != cn->end_group_list.end(); ++i) {
	HRTRUE(count < ngroup_end);
	group_end[count++] = *i;
      }

    error_out:
      return ret;
    }

    int transitionsFromCompileNode(CompileNode *cn, size_t n_indicies) {
      int ret = 0;
      transitions = 
	static_cast<Node **>(calloc(sizeof(Node*), n_indicies));
      HRNULL(transitions);

      for (typename ReferenceValueMap<int, CompileNode *>::iterator i =
	     cn->transitions.begin();
	   i != cn->transitions.end();
	   ++i) {
	HRTRUE(i->first < (off_t)n_indicies);
	CompileNode *dest = i->second;
	while (dest->replaced_by != NULL) {
	  dest = dest->replaced_by;
	}
	transitions[i->first] = dest->node;
      }
    error_out:
      return ret;
    }

    Node *getTransition(int index) {
      if (index < 0) {
	return NULL;
      }
      return transitions[index];
    }

  } *nodes;

  typedef list<pair<off_t, int> > GroupList;

  class CGI: virtual public Base {
  private:
    MVAR(public, CompileNode, group_begin);
    MVAR(public, CompileNode, last_or_end);

    int gn_;
  public:
    explicit CGI() {}
    virtual ~CGI() {}
    int gn() const { return gn_; }
    void set_gn(int n) { gn_ = n; }
  };

  int ngroups; // number of groups.
  int nnodes;
  int nindicies;
  map <C, int> index_map;
  int ngroup_end;  // number of groups that end here.
  off_t *group_end;

  // state used for compiling;
  bool compiling;
  CompileNode *compile_current_node;
  ReferenceList<CGI *> group_starts;
  CompileNode *last_group_begin;
  CompileNode *last_or_node;
  ReferenceList<CompileNode *> all_nodes;
  int redundant_node_count;
  GroupList pending_end_group;
  map<CompileNode *, list<CompileNode *> > empty_transitions;
  int empty_transition_count;

protected:
  CompileNode *newCompileNode() {
    int ret = 0;
    CompileNode *cn;
    cn = new CompileNode();
    HRNULL(cn);
    all_nodes.push_back(cn);
    HRC(compileAddNullTransition(cn, compile_nodes));
  error_out:
    if (ret) {
      if (cn) {
	all_nodes.erase(cn);
	delete cn;
	cn = NULL;
      }
    }
    return cn;
  }

  int charToIndex(C ch) {
    if (index_map.count(ch) != 0) {
      return index_map[ch];
    }
    if (compiling) {
      index_map[ch] = nindicies++;
      return index_map[ch];
    }

    return -1;
  }

  set<C> getUniverse() {
    static set<C> suniverse;
    static bool initialized = false;
    // single threaded. only used during compile.
    if (initialized) {
      return suniverse;
    }

    /* populate with all valid characters.  
       this is really bad if C is more than
       a short and won't work if it's not an
       integer type. */
    C i = 0;
    do {
      suniverse.insert(i);
      i++;
    } while  (i != 0 );

    return suniverse;
  }

  int startMatches(ReferenceList<Match *> *matches, off_t start) {
    int ret = 0;
    AUTODEREF(Match *, m);
    m = new Match(ngroups);
    HRNULL(m);
    m->begin = start;
    matches->push_back(m);
  error_out:
    return ret;
  }

  int restartMatches(ReferenceList<Match *> *matches, off_t start) {
    int ret = 0;
    matches->clear();
    AUTODEREF(Match *, m);
    m = new Match(ngroups);
    HRNULL(m);
    m->begin = start;
    matches->push_back(m);
  error_out:
    return ret;
  }

  int endMatch(ReferenceList<Match *> *matches, off_t end) {
    int ret = 0;
    foreach(i, (*matches)) {
      HRTRUE((*i)->group_stack.size() == 0);
      (*i)->end = end;
      for (size_t j = 0; j < (*i)->ngroups; j++) {
	for (list<Match::group_type>::iterator k =
	       (*i)->group_matches[j].begin();
	     k != (*i)->group_matches[j].end(); ++k) {
	  if (k->second == 0) {
	    k->second = k->first;
	  }
	}
      }
    }
  error_out:
    return ret;

  }

  int beginGroup(Node *next, ReferenceList<Match *> *matches,
		 off_t pos) {
    int ret = 0;
    foreach(i, (*matches)) {
      for (int j = 0; j < next->ngroup_begin; ++j) {
	off_t gn = next->group_begin[j];
	(*i)->group_matches[gn].push_back(Match::group_type(pos, 0));
      }
    }
    return ret;
  }

  int endGroup(Node *next, ReferenceList<Match *> *matches,
	       off_t pos) {
    int ret = 0;
    foreach(i, (*matches)) {
      for (int j = 0; j < next->ngroup_end; ++j) {
	off_t gn = next->group_end[j];
	int bias = next->group_bias[j];
	for (list<pair<off_t, off_t> >::iterator k = 
	       (*i)->group_matches[gn].begin(); 
	     k != (*i)->group_matches[gn].end();
	     ++k) {
	  if (k->second <= 0) {
	    if (bias == -128) {
	      k->first = 0;
	      k->second = bias;
	    } else {
	      k->second = pos + bias;
	    }
	  }
	}
	(*i)->group_matches[gn].remove_if(pair_equals<off_t, 0, int, -128>);
      }
    }
    return ret;
  }

  int transition(Node *from, Node *to, ReferenceList<Match *> *matches, 
		 off_t pos) {
    int ret = 0;
    if (to == nodes) {
      HRC(restartMatches(matches, pos));
    }

    /* We have to end first to deal with loops ending a group before
       it begins. */
    if (to->ngroup_end != 0) {
      HRC(endGroup(to, matches, pos));
    }

    if (to->ngroup_begin != 0) {
      HRC(beginGroup(to, matches, pos));
    }
  error_out:
    return ret;
  }

  CompileNode *compileGetCurrentGroupBegin() {
    int ret = 0;
    if (group_starts.size() == 0) {
      return NULL;
    }
    HRTRUE(group_starts.back() > 0);
    return group_starts.back()->group_begin();
  error_out:
    return NULL;
  }

  CompileNode *compileGetLastGroupBegin() {
    return last_group_begin;
  }

  int compileOr() {
    int ret = 0;
    CompileNode *n = compileGetCurrentGroupBegin();
    if (n == NULL) {
      n = compile_nodes;
      compile_current_node->accepting = true;
    }

    if (last_or_node == NULL) {
      last_or_node = compile_current_node;
    } else {
      HRC(compileMergeNodes(compile_current_node, last_or_node));
    }

    HRC(compileAddNullTransition(compile_current_node, n));
    compile_current_node = n;

  error_out:
    return ret;
  }

  int compileEndGroup() {
    int ret = 0;
    CGI *c = NULL;
    if (last_or_node != NULL) {
      HRC(compileMergeNodes(compile_current_node, last_or_node));
      compile_current_node = last_or_node;
    }

    HRTRUE(group_starts.size() > 0);
    last_group_begin = group_starts.back()->group_begin();
    last_or_node = group_starts.back()->last_or_end();
    c = group_starts.back();
    pending_end_group.push_back(pair<off_t, int>(c->gn(), -1));
    group_starts.pop_back();
  
  error_out:
    return ret;
  };

  int compileBeginGroup() {
    int ret = 0;
    AUTODEREF(CGI *, bg);
    set<CompileNode *> seen_nodes;
    bg = new CGI();
    HRNULL(bg);
    bg->set_group_begin(compile_current_node);
    bg->set_last_or_end(last_or_node);
    bg->set_gn(ngroups);
    group_starts.push_back(bg);

    last_or_node = NULL;

    /* all existing transitions should not start the group, so
       we put a special delete marker at the end of them. */

    for (typename list<CompileNode *>::iterator i = 
	   empty_transitions[compile_current_node].begin();
	 i != empty_transitions[compile_current_node].end();
	 ++i) {
      (*i)->end_group_list.push_back(ngroups);
      (*i)->bias_group_list.push_back(-128);
      (*i)->ngroup_end++;
    }


    for (typename ReferenceValueMap<int, CompileNode *>::iterator i =
	   compile_current_node->transitions.begin();
	 i != compile_current_node->transitions.end();
	 ++i) {
      if (seen_nodes.count(i->second) > 0) {
	continue;
      }
      seen_nodes.insert(i->second);
      i->second->end_group_list.push_back(ngroups);
      i->second->bias_group_list.push_back(-128);
      i->second->ngroup_end++;
    }

    compile_current_node->begin_group_list.push_back(ngroups++);
    compile_current_node->ngroup_begin++;

  error_out:
    return ret;
  }

  int handlePendingGroupBegins(GroupList &begin_group, int bias) {
    for (list<pair<off_t, int> >::iterator i = begin_group.begin();
	 i != begin_group.end(); ++i) {
      compile_current_node->begin_group_list.push_back(i->first);
      compile_current_node->ngroup_begin++;
    }
    begin_group.clear();
    return 0;
  }

  int compileAddSetTransition(set<C> chrs) {
    int ret = 0;
    AUTODEREF(CompileNode *, cn);
    cn = newCompileNode();
    HRNULL(cn);

    last_group_begin = compile_current_node;

    for (typename set<C>::iterator i = chrs.begin(); i != chrs.end(); ++i) {
      int ind = charToIndex(*i);
      HRTRUE (ind >= 0);
      if (compile_current_node->transitions.count(ind) == 0) {
	compile_current_node->transitions[ind] = cn;
	HRNULL(compile_current_node->transitions[ind]);
	HRC(compileNodeAdded(ind));
      } else {
	CompileNode *n = compile_current_node->transitions[ind];
	while (n->replaced_by != NULL) {
	  n = n->replaced_by;
	}
	HRC(compileAddNullTransition(cn, n));
      }
    }

    compile_current_node = cn;

    if (pending_end_group.size() > 0) {
      HRC(handlePendingGroupEnds(pending_end_group, 0));
    }

  error_out:
    return ret;
  }

  int compileAddAnyTransition() {
    int ret = 0;
    set<C> u = getUniverse();
    HRC(compileAddSetTransition(u));
  error_out:
    return ret;
  }

  int compileAddStarTransition() {
    int ret = 0;
    CompileNode *n = compileGetLastGroupBegin();
    if (n == NULL) {
      n = compile_nodes;
    }
    HRC(compileMergeNodes(compile_current_node, n));
    compile_current_node = n;

  error_out:
    return ret;
  }

  int compileAddPlusTransition() {
    int ret = 0;
    CompileNode *n = compileGetLastGroupBegin();
    if (n == NULL) {
      n = compile_nodes;
    }
    HRC(compileAddNullTransition(compile_current_node, n));

  error_out:
    return ret;

  }

  int compileAddEOLTransition() {
    int ret = 0;
    HRC(compileAddTransitionPostTranslate(END_OF_STRING_INDEX));

  error_out:
    return ret;
  }

  int compileAddBOLTransition() {
    int ret = 0;
    HRC(compileAddTransitionPostTranslate(BEGINNING_OF_STRING_INDEX));

  error_out:
    return ret;
  }

  int compileAddNullTransition(CompileNode *from, CompileNode *to) {
    int ret = 0;
    if (from == NULL || to == NULL) {
      goto error_out;
    }

    while (to->replaced_by != NULL) {
      to = to->replaced_by;
    }

    while (from->replaced_by != NULL) {
      from = from->replaced_by;
    }

    if (from == to) {
      goto error_out;
    }

    if (to->empty_transitions.contains(from)) {
      goto error_out;
    }

    empty_transitions[from].push_back(to);

    to->empty_transitions.push_back(from);
    empty_transition_count++;

    for (list<off_t>::iterator i = from->begin_group_list.begin();
	 i != from->begin_group_list.end(); ++i) {
      if (listext<off_t>::contains(to->end_group_list, *i)) {
	to->end_group_list.push_back(*i);
	to->bias_group_list.push_back(-128);
	to->ngroup_end++;
      }
      
    }

  error_out:
    return ret;
  }

  // n2 is dominant.  n1 is assumed to only
  // have one reference and be newly created.
  int compileMergeNodes(CompileNode *n1, CompileNode *n2) {
    if (n1 != n2) {
      // We will pick this up when we 
      // reduce everything.
      n1->replaced_by = n2;
      redundant_node_count ++;
      n2->ngroup_end += n1->ngroup_end;
      n2->ngroup_begin += n1->ngroup_begin;
      for (list<off_t>::iterator i = n1->begin_group_list.begin();
	   i != n1->begin_group_list.end(); ++i) {
	n2->begin_group_list.push_back(*i);
      }
      for (list<off_t>::iterator i = n1->end_group_list.begin();
	   i != n1->end_group_list.end(); ++i) {
	n2->end_group_list.push_back(*i);
	n2->bias_group_list.push_back(0);
      }
    }
    return 0;
  }

  // follow any empty transitions
  // and add the node there as well.
  int compileNodeAdded(int ind) {
    return 0;
  }


  int compileAddTransitionPostTranslate(int ind) {
    int ret = 0;
    HRTRUE (ind >= 0);
    if (compile_current_node->transitions.count(ind) == 0) {
      AUTODEREF(CompileNode *, cn);
      cn = newCompileNode();
      HRNULL(cn);

      compile_current_node->transitions[ind] = cn;
      HRNULL(compile_current_node->transitions[ind]);
      HRC(compileNodeAdded(ind));
    }

    last_group_begin = compile_current_node;

    compile_current_node = compile_current_node->transitions[ind];
    while (compile_current_node->replaced_by != NULL) {
      compile_current_node = compile_current_node->replaced_by;
    }

    if (pending_end_group.size() > 0) {
      HRC(handlePendingGroupEnds(pending_end_group, 0));
    }

  error_out:
    return ret;
  }

  int compileAddTransition(C ch) {
    int ret = 0;

    int ind = charToIndex(ch);

    HRTRUE(ind >= 0);

    HRC(compileAddTransitionPostTranslate(ind));

  error_out:
    return ret;
  }

  int initNodes() {
    return 0;
  }

  int compileBegin() {
    int ret=0;

    compile_nodes = new CompileNode();
    HRNULL(compile_nodes);
    all_nodes.push_back(compile_nodes);

    compile_current_node = compile_nodes;

  error_out:
    return ret;
  }

  int handlePendingGroupEnds(GroupList &end_group, int bias) { 
    for (list < pair<off_t, int> >::iterator i = end_group.begin();
	 i != end_group.end(); ++i) {
      compile_current_node->end_group_list.push_back(i->first);
      compile_current_node->bias_group_list.push_back(i->second + bias);
      compile_current_node->ngroup_end++;
    }
    end_group.clear();
    return 0;
  }

  CompileNode *cloneNode(CompileNode *old) {
    int ret = 0;
    CompileNode *n = newCompileNode();
    if (n == NULL) {
      return NULL;
    }

    for (typename ReferenceValueMap<int, CompileNode *>::iterator i =
	   old->transitions.begin(); 
	 i != old->transitions.end();
	 ++i) {
      n->transitions[i->first] = i->second;
    }

    n->begin_group_list = old->begin_group_list;
    n->end_group_list = old->end_group_list;
    n->bias_group_list = old->bias_group_list;
    n->ngroup_begin = old->ngroup_begin;
    n->ngroup_end = old->ngroup_end;
    n->accepting = old->accepting;

    if (empty_transitions.count(old) > 0) {
      for (typename ReferenceList<CompileNode *>::iterator i = 
	     empty_transitions[old].begin();
	   i != empty_transitions[old].end();
	   ++i) {
	HRC(compileAddNullTransition(n, *i));
      }
    }

    old ->clone = n;

  error_out:

    return n;
  }

  int compileMergeNullTransitions() {
    int ret = 0;

    while (empty_transition_count > 0) {
      for (typename ReferenceList<CompileNode *>::iterator i =
	     all_nodes.begin();
	   i != all_nodes.end(); ++i) {
	for (typename ReferenceList<CompileNode *>::iterator j = 
	       (*i)->empty_transitions.begin(); 
	     j != (*i)->empty_transitions.end(); ++j) {
	  for (typename ReferenceValueMap<int, CompileNode *>::iterator k =
		 (*i)->transitions.begin();
	       k != (*i)->transitions.end(); ++k) {
	    if ((*j)->transitions.count(k->first) == 0) {
	      if (k->second->clone == NULL && (*j)->ngroup_begin) {
		AUTODEREF(CompileNode *, cn);
		cn = cloneNode(k->second);
		HRNULL(cn);
		(*j)->transitions[k->first] = cn;
		for (list<off_t>::iterator l = (*j)->begin_group_list.begin();
		     l != (*j)->begin_group_list.end(); ++l) {
		  cn->end_group_list.push_back(*l);
		  cn->bias_group_list.push_back(-128);
		  cn->ngroup_end++;
		}
	      } else {
		if (k->second->clone == NULL) {
		  (*j)->transitions[k->first] = k->second;
		} else {
		  (*j)->transitions[k->first] = k->second->clone;
		}
	      }
	    } else {
	      if (!(*i)->accepting) {
		HRC(compileAddNullTransition((*i)->transitions[k->first],
					     (*j)->transitions[k->first]));
	      }
	    }
	  }
	  empty_transition_count--;
	}
	(*i)->empty_transitions.clear();
      }
    }
    
  error_out:
    return ret;
  }

  int compileEnd() {
    int ret =0;
    int ind=0;
    size_t node_count;

    HRTRUE(group_starts.size() == 0);

    if (pending_end_group.size() > 0) {
      HRC(handlePendingGroupEnds(pending_end_group, 1));
    }

    compile_current_node->accepting = true;

    HRC(compileMergeNullTransitions());

    node_count = all_nodes.size() - redundant_node_count;

    nodes = new Node[node_count];
    HRNULL(nodes);

    for (typename ReferenceList<CompileNode *>::iterator i = 
	   all_nodes.begin();
	 i != all_nodes.end(); ++i) {
      if ((*i)->replaced_by != NULL) {
	continue;
      }
      HRC(nodes[ind++].fromCompileNode(*i));
    }

    for (typename ReferenceList<CompileNode *>::iterator i =
	   all_nodes.begin();
	 i != all_nodes.end(); ++i) {
      if ((*i)->replaced_by != NULL) {
	continue;
      }
      HRC((*i)->node->transitionsFromCompileNode(*i, nindicies));
    }

    compile_current_node = NULL;
    group_starts.clear();
    last_group_begin = NULL;
    last_or_node = NULL;
    all_nodes.clear();
    pending_end_group.clear();
    
  error_out:
    return ret;
  }

public:
  explicit RegExp(): ngroups(0), nnodes(0), nindicies(2), compiling(false),
		     compile_current_node(NULL),
		     last_group_begin(NULL), last_or_node(NULL), 
		     redundant_node_count(0), empty_transition_count(0) {}
  virtual ~RegExp() {}
  bool match(basic_string<C> str, ReferenceList<Match *> *matches = NULL) {
    int ret = 0;
    bool deleteMatches=false;
    Node *next=NULL;
    Node *cur=NULL;
    size_t len;
    size_t ind = 0;

    HRNULL(nodes);
    if (matches == NULL) {
      deleteMatches = true;
      matches = new ReferenceList<Match *>();
      HRNULL(matches);
    }
    len = str.length();
    HRC(startMatches(matches, 0));
    cur = nodes;
    if (cur->accepting) {
      goto error_out;
    }
    next = cur->getTransition(BEGINNING_OF_STRING_INDEX); 
    if (next == NULL) {
      next = nodes;
    }


    HRC(transition(cur, next, matches, 0));
    if (next->accepting) {
      goto error_out;
    }

    for (ind = 0; ind < len; ++ind) {
      C ch = str[ind];
      int index = charToIndex(ch);
      Node *next = cur->getTransition(index);
      if (next == NULL) {
	next = nodes;
      }
      HRC(transition(cur, next, matches, ind + 1));
      if (next->accepting) {
	goto error_out;
      }
      cur = next;
    }

    next = cur->getTransition(END_OF_STRING_INDEX); 
    if (next == NULL) {
      next = nodes;
    }
    HRC(transition(cur, next, matches, len + 1));
    if (next->accepting) {
      goto error_out;
    }

    ret = -1;
    
  error_out:
    if (ret == 0) {
      endMatch(matches, ind + 1);
    }
    if (deleteMatches && matches != NULL) {
      delete matches;
    }
    return ret == 0;
  }

  int compile(basic_string<C> re) {
    int ret = 0;
    set<C> chrs;
    C last_set_char = 0;
    bool setSawDash = false;
    bool escape = false;
    bool inset = false;
    Synchronized(this);
    compiling = true;
    HRC(initNodes());
    HRC(compileBegin());

    for (size_t i = 0; i < re.length(); ++i) {
      if (escape) {
	HRC(compileAddTransition(re[i]));
	escape = false;
      } else  if (inset) {
	if (setSawDash) {
	  for (C ch = last_set_char; ch <= re[i]; ++ch) {
	    chrs.insert(ch);
	  }
	  last_set_char = 0;
	  setSawDash = false;
	} else if (re[i] == ']') {
	  inset = false;
	  compileAddSetTransition(chrs);
	  chrs.clear();
	} else {
	  if (re[i] == '-' && last_set_char != 0) {
	    setSawDash = true;
	  } else {
	    chrs.insert(re[i]);
	    last_set_char = re[i];
	  }
	}
      } else {
	switch(re[i]) {
	case '\\':
	  escape = true;
	  break;

	case ')':
	  HRC(compileEndGroup());
	  break;

	case '(':
	  HRC(compileBeginGroup());
	  break;

	case '|':
	  HRC(compileOr());
	  break;

	case '[':
	  inset=true;
	  break;

	case '.':
	  HRC(compileAddAnyTransition());
	  break;

	case '*':
	  HRC(compileAddStarTransition());
	  break;

	case '+':
	  HRC(compileAddPlusTransition());
	  break;

	case '$':
	  HRC(compileAddEOLTransition());
	  break;

	case '^':
	  HRC(compileAddBOLTransition());
	  break;
	
	default:
	  HRC(compileAddTransition(re[i]));
	  break;
	}
      }
    }
    HRC(compileEnd());

  error_out:
    compiling = false;
    return ret;
  }

  
};

}} //namespace


#endif //TEXTUS_UTIL_REGEXP_H_
