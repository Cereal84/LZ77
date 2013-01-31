/**
 * @file tree.h
 *
 * Binary Search Tree implemented using an an array where each entry is a node.
 * A node is virtually a sub string, of length "lookahead", of the window (dictionary).
 * Virtually because in the node not exist a string but it's index represent the position
 * of the first character, of the substring, in the window.
 *
 * When build the tree we've a node call ROOT.
 *
 *
 * The nodes are memorized in a circular array, given the index of the substring in the window it's
 * index in the array is given by
 *
 *		(index_string MOD look_ahead_length) + 1
 *
 * The plus one it's used to avoid to replace the ROOT.
 *
 * @author Pischedda Alessandro
 */

#include "../include/tree.h"


struct Node* build_tree(int length)
{
	if(length <= 0){
		errno = EINVAL;
		return NULL;
	}
	// set automatically all nodes as UNUSED
	return calloc( (length + 1), sizeof(struct Node) );

}

void add_node( struct Node *tree, int new_node, const struct window *w)
{
	int current_node;
	int win_position;
	int new_node_position;

	new_node_position = ( new_node % w->window_length ) +1; // +1 because of ROOT node

   	// is it the "real" root ?
	if( tree[ ROOT ].greater == UNUSED)
	{
		tree[ ROOT ].greater = new_node_position;
		tree[ new_node_position ].father = ROOT;
		tree[ new_node_position ].position = new_node;
		return;
	}

    	// well the root alredy exsist so find a place for the new node
	current_node = tree[ ROOT ].greater;

	for(;;)
	{
		win_position = tree[ current_node ].position;

        	// the new node is smaller than current_node, so go left
		if ( window_cmp(w, win_position, new_node) > 0 )
		{

            		// free location ?
			if( tree[ current_node ].smaller == UNUSED)
			{
				tree[ current_node ].smaller = new_node_position;
				break;

			}
			current_node = tree[ current_node ].smaller;

		}else{ // the new node is greater than current_node so go right

			if(tree[ current_node ].greater == UNUSED)
			{
				tree[ current_node ].greater = new_node_position;
				break;
			}
			current_node = tree[ current_node ].greater;

		}


	}

	tree[ new_node_position ].father = current_node;
	tree[ new_node_position ].position = new_node;
	tree[ new_node_position ].greater = UNUSED;
	tree[ new_node_position ].smaller = UNUSED;

}



void level_up(struct Node *tree, int old_node, int new_node )
{

	tree[ new_node ].father = tree[ old_node ].father;
	// Update the old_node's father info.
        if( tree[ tree[ old_node ].father ].smaller == old_node )
		tree[ tree[ old_node ].father ].smaller = new_node;
	else
		tree[ tree[ old_node ].father ].greater = new_node;

	tree[ old_node ].father = UNUSED;
	tree[ old_node ].smaller = UNUSED;
	tree[ old_node ].greater = UNUSED;
	// position should be unused too
}


void replace_node(struct Node *tree, int old_node, int new_node)
{
	int father;
	int greater;

	// Update info old_node's father
	father = tree[ old_node ].father;
	if ( tree[ father ].greater == old_node )
		tree[ father ].greater = new_node;
	else
		tree[ father ].smaller = new_node;

	// update tree[ old_node ].greater info.
	greater = tree[ old_node ].greater;
	tree[ greater ].father = new_node;

	if( tree[ old_node ].smaller == new_node)
		tree[ new_node ].father = father;
	else{ // must update info about new_node's left-son and father

		int smaller;
		int father_new_node;

		smaller = tree[ old_node ].smaller;
		tree[ smaller ].father = new_node;

		if( tree[ new_node ].smaller != UNUSED ){

			smaller = tree[new_node].smaller;
			tree[ smaller ].father = tree[ new_node ].father;

		}
		father_new_node = tree[new_node].father;
		tree[father_new_node].greater = tree[new_node].greater;
		tree[ new_node ].smaller = tree[ old_node ].smaller;
	}


	// update new_node's information
	tree[ new_node ].greater = tree[ old_node ].greater; // right son
	tree[ new_node ].father = tree[ old_node ].father;

	// "delete" old_node
	tree[ old_node ].father = UNUSED;
	tree[ old_node ].smaller = UNUSED;
	tree[ old_node ].greater = UNUSED;

}

int find_max_son(struct Node *tree, int node){

	int greatest = tree[node].smaller;
	while ( tree[greatest].greater != UNUSED)
		greatest = tree[greatest].greater;

	return greatest;

}

void delete_node( struct Node *tree, int node, int n_nodes )
{
	int node_position;

	node_position = ( node % n_nodes ) +1; // +1 because of ROOT node

	// it's a leaf
	if( tree[ node_position ].smaller == UNUSED && tree[ node_position ].greater == UNUSED)
	{
		//printf("delete leaf - node %d\n",node_position);
		int father = tree[node_position].father;

		// update father informations		
		if(tree[father].smaller == node_position)
			tree[father].smaller = UNUSED;
		else
			tree[father].greater = UNUSED;
		// update node informations
		tree[node_position].father = UNUSED;
		return;
	}

	if ( tree[ node_position ].greater == UNUSED )
		level_up(tree, node_position, tree[ node_position ].smaller );

	else if	( tree[ node_position ].smaller == UNUSED )
		level_up(tree, node_position, tree[ node_position ].greater );

	else{ // replace the node with the max left son
		//printf("replace ");
		int max_son;
		max_son = find_max_son(tree, node_position);
		//printf(" %d with %d\n",node_position,max_son);
		replace_node(tree, node_position, max_son);
	}

}

void empty_tree(struct Node *tree, int n_nodes){
	int i;

	tree[ROOT].greater = UNUSED;
	for(i = 0; i<=n_nodes; i++){
		tree[i].father = UNUSED;
		tree[i].smaller = UNUSED;
		tree[i].greater = UNUSED;
		tree[i].position = UNUSED;
	}
}



int window_cmp(const struct window *w, int s1_head, int s2_head)
{
	int i;
	int s1_i, s2_i;
	int diff;

	diff=0; 
	
	for(i = 0; i < w->look_ah_length; i++){
		s1_i = wrap( s1_head + i, w->window_length + w->dict_position , w->dict_position);
		s2_i = wrap( s2_head + i, w->window_length + w->dict_position , w->dict_position);

		diff = w->window[ s1_i ] - w->window[ s2_i ];
		if (diff != 0)
			break;
	}

	return diff;
}



