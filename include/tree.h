/**
 * @file tree.h
 *
 * This file contains the structure data and all the functions needed to create and
 * handle a binary tree.
 * 
 * @author Pischedda Alessandro
 */


#ifndef _TREE_H_
#define _TREE_H_

#include "window.h"

#define ROOT 0
#define UNUSED 0


/** @struct Node
 *
 * Contain the definition of the node structure that is used to build the tree.
 *  - position	is the absolute position in the window array
 *  - smaller	is the left son
 *  - greater	is the right son
 *  - father	the father of this node
 *
 */
struct Node{
	int position;
	int smaller;
	int greater;
	int father;
};


/**
 * Build and initialize the tree.
 * Using the calloc and the definition of UNUSED the initialization is automatic. 
 *
 * @param length	it's the number of nodes
 * 
 * @return		struct Node pointer
 *			NULL in case of error
 * ERRORS
 *	EINVAL		if some function's arguments isn't correct.
 *	Others		are all the possible error returned by calloc() function.
 */
struct Node* build_tree( int length);


/**
 * Add node in the Tree.
 *
 * @param tree	tree where add the node
 * @param node	value of node to add
 * @param window 
 */
void add_node( struct Node *tree, int node, const struct window *window);


/**
 * Delete a node from the Tree.
 *
 * @param tree		tree from where remove the node
 * @param node		is the position of the string in the window array
 * @param win_length	window length, used to calculate modulo 
 *
 */
void delete_node(struct Node *tree, int node, int win_length);

/**
 * Used in function delete_node() when one of the sons of the node to delete is UNUSED.
 * Take the son and use it to replace the node.
 *
 * @param tree	tree structure
 * @param node	node to be replace
 * @param son 	son who replace the node
 */
void level_up(struct Node *tree, int node, int son);

/**
 * Find the bigger left son of a node. This function is used by the delete function.
 * It's a left son in order to don't touch the right part of the tree.
 *
 * @param tree	tree strcuture
 * @param node	node from where search the bigger left son
 * @return	the index of the greater son of the node "node"
 */
int find_max_son(struct Node *tree, int node);

/**
 * Replace old_node with the new_node
 *
 * @param old_node : to be replaced
 * @param new_node : node replace with.
 */
void replace_node(struct Node *tree, int old_node, int new_node);

/**
 * All the field of all node will be set UNUSED.
 * Is usefull when you have to "delete" all nodes without using delete_node().
 *
 * @param tree : tree structure to fill with zeros
 * @param n_nodes : the number of nodes in the tree
 */
void empty_tree(struct Node *tree, int n_nodes);


/**
 * Similar to strncmp, except it can compares strings that land across the 
 * sliding window. It use the function wrap().
 *
 * @param w  : window structure
 * @param s1 : position of the first character of the first string
 * @param s2 : position of the first character of the second string
 *
 * @return : positive value if s1 > s2, negative value if s1 < s2 and 0 if s1 == s2
 */
int window_cmp(const struct window *w, int s1, int s2);



#endif

