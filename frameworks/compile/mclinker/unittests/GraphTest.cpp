//===- GraphTest.cpp ------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "GraphTest.h"
#include "mcld/ADT/GraphLite/Digraph.h"
#include "mcld/ADT/GraphLite/ListDigraph.h"

using namespace mcld;
using namespace mcld::test;
using namespace mcld::graph;

// Constructor can do set-up work for all test here.
GraphTest::GraphTest() {
}

// Destructor can do clean-up work that doesn't throw exceptions here.
GraphTest::~GraphTest() {
}

// SetUp() will be called immediately before each test.
void GraphTest::SetUp() {
}

// TearDown() will be called immediately after each test.
void GraphTest::TearDown() {
}

//===----------------------------------------------------------------------===//
// Testcases
//===----------------------------------------------------------------------===//
TEST_F(GraphTest, list_digraph_add_n_erase_nodes_1) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(u2 == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(u3 == u2->prev);
  ASSERT_TRUE(u1 == u2->next);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(u2 == u3->next);
  ASSERT_TRUE(NULL == u3->prev);

  ListDigraph::Node* head = NULL;
  graph.getHead(head);
  ASSERT_TRUE(head == u3);

  graph.erase(*u2);

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(u3 == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(u1 == u3->next);
  ASSERT_TRUE(NULL == u3->prev);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(NULL == u2->prev);
  ASSERT_TRUE(NULL == u2->next);

  graph.getHead(head);
  ASSERT_TRUE(head == u3);
}

TEST_F(GraphTest, list_digraph_add_n_erase_nodes_2) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(u2 == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(u3 == u2->prev);
  ASSERT_TRUE(u1 == u2->next);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(u2 == u3->next);
  ASSERT_TRUE(NULL == u3->prev);

  ListDigraph::Node* head = NULL;
  graph.getHead(head);
  ASSERT_TRUE(head == u3);

  graph.erase(*u1);

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(NULL == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(u3 == u2->prev);
  ASSERT_TRUE(NULL == u2->next);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(u2 == u3->next);
  ASSERT_TRUE(NULL == u3->prev);

  graph.getHead(head);
  ASSERT_TRUE(head == u3);
}

TEST_F(GraphTest, list_digraph_add_n_erase_nodes_3) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(u2 == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(u3 == u2->prev);
  ASSERT_TRUE(u1 == u2->next);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(u2 == u3->next);
  ASSERT_TRUE(NULL == u3->prev);

  ListDigraph::Node* head = NULL;
  graph.getHead(head);
  ASSERT_TRUE(head == u3);

  graph.erase(*u3);

  ASSERT_TRUE(NULL == u3->first_in);
  ASSERT_TRUE(NULL == u3->first_out);
  ASSERT_TRUE(NULL == u3->prev);
  ASSERT_TRUE(NULL == u3->next);

  ASSERT_TRUE(NULL == u1->first_in);
  ASSERT_TRUE(NULL == u1->first_out);
  ASSERT_TRUE(u2 == u1->prev);
  ASSERT_TRUE(NULL == u1->next);

  ASSERT_TRUE(NULL == u2->first_in);
  ASSERT_TRUE(NULL == u2->first_out);
  ASSERT_TRUE(u1 == u2->next);
  ASSERT_TRUE(NULL == u2->prev);

  graph.getHead(head);
  ASSERT_TRUE(head == u2);
}

TEST_F(GraphTest, list_digraph_add_arcs_1) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u2);
  ListDigraph::Arc* a2 = graph.addArc(*u2, *u3);
  ListDigraph::Arc* a3 = graph.addArc(*u3, *u1);

  ASSERT_TRUE(u1 == a1->source && u2 == a1->target);
  ASSERT_TRUE(u2 == a2->source && u3 == a2->target);
  ASSERT_TRUE(u3 == a3->source && u1 == a3->target);

  ASSERT_TRUE(u1->first_in == a3 && u1->first_out == a1);
  ASSERT_TRUE(u2->first_in == a1 && u2->first_out == a2);
  ASSERT_TRUE(u3->first_in == a2 && u3->first_out == a3);
}

TEST_F(GraphTest, list_digraph_add_arcs_2) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u1);
  ListDigraph::Arc* a2 = graph.addArc(*u1, *u2);
  ListDigraph::Arc* a3 = graph.addArc(*u1, *u3);

  ASSERT_TRUE(u1 == a1->source && u1 == a1->target);
  ASSERT_TRUE(u1 == a2->source && u2 == a2->target);
  ASSERT_TRUE(u1 == a3->source && u3 == a3->target);

  ASSERT_TRUE(u1->first_in == a1 && u1->first_out == a3);
  ASSERT_TRUE(u2->first_in == a2 && u2->first_out == NULL);
  ASSERT_TRUE(u3->first_in == a3 && u3->first_out == NULL);
}

TEST_F(GraphTest, list_digraph_add_n_erase_arcs_1) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u2);
  ListDigraph::Arc* a2 = graph.addArc(*u2, *u3);
  ListDigraph::Arc* a3 = graph.addArc(*u3, *u1);

  graph.erase(*a2);

  ASSERT_TRUE(u1 == a1->source && u2 == a1->target);
  ASSERT_TRUE(u3 == a3->source && u1 == a3->target);

  // remove from the fan-out list
  ASSERT_TRUE(u2->first_in == a1 && u2->first_out == NULL);

  // remove from the fan-in list
  ASSERT_TRUE(u3->first_in == NULL && u3->first_out == a3);

  // put into free list
  ASSERT_TRUE(NULL == a2->next_in);
}

TEST_F(GraphTest, list_digraph_add_n_erase_arcs_2) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u2);
  ListDigraph::Arc* a2 = graph.addArc(*u2, *u3);
  ListDigraph::Arc* a3 = graph.addArc(*u3, *u1);

  graph.erase(*a1);

  ASSERT_TRUE(u2 == a2->source && u3 == a2->target);
  ASSERT_TRUE(u3 == a3->source && u1 == a3->target);

  // remove from the fan-out list
  ASSERT_TRUE(u1->first_in == a3 && u1->first_out == NULL);

  // remove from the fan-in list
  ASSERT_TRUE(u2->first_in == NULL && u2->first_out == a2);

  // put into free list
  ASSERT_TRUE(NULL == a1->next_in);
}

TEST_F(GraphTest, list_digraph_add_n_erase_arcs_3) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u2);
  ListDigraph::Arc* a2 = graph.addArc(*u2, *u3);
  ListDigraph::Arc* a3 = graph.addArc(*u3, *u1);

  graph.erase(*a3);

  ASSERT_TRUE(u1 == a1->source && u2 == a1->target);
  ASSERT_TRUE(u2 == a2->source && u3 == a2->target);

  // remove from the fan-out list
  ASSERT_TRUE(u1->first_in == NULL && u1->first_out == a1);

  // remove from the fan-in list
  ASSERT_TRUE(u3->first_in == a2 && u3->first_out == NULL);

  // put into free list
  ASSERT_TRUE(NULL == a3->next_in);
}

TEST_F(GraphTest, list_digraph_add_n_erase_arcs_4) {
  ListDigraph graph;

  ListDigraph::Node* u1 = graph.addNode();
  ListDigraph::Node* u2 = graph.addNode();
  ListDigraph::Node* u3 = graph.addNode();

  ListDigraph::Arc* a1 = graph.addArc(*u1, *u1);
  graph.addArc(*u1, *u2);
  graph.addArc(*u1, *u3);

  graph.erase(*u1);

  ASSERT_TRUE(u2->first_in == NULL);
  ASSERT_TRUE(u3->first_in == NULL);
  ASSERT_TRUE(a1->next_in == NULL);
}

TEST_F(GraphTest, api_test) {
  Digraph graph;

  Digraph::Node node = graph.addNode();
  graph.addNode();
  graph.addNode();
  graph.addNode();

  ASSERT_EQ(4, graph.numOfNodes());
  ASSERT_EQ(0, graph.numOfArcs());
}
