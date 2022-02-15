#include "bptree.h"
#include <sys/time.h>

void print_tree_core(NODE *n)
{
	printf("[");
	for (int i = 0; i < n->nkey; i++)
	{
		if (!n->isLeaf)
			print_tree_core(n->chi[i]);
		printf("%d", n->key[i]);
		if (i != n->nkey - 1 && n->isLeaf)
			putchar(' ');
	}
	if (!n->isLeaf)
		print_tree_core(n->chi[n->nkey]);
	printf("]");
}

void print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n");
	fflush(stdout);
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE))))
		ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

NODE *
alloc_internal(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE))))
		ERR;
	node->isLeaf = false;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

NODE *
alloc_root(NODE *left, int rs_key, NODE *right)
{
	NODE *node;

	if (!(node = (NODE *)calloc(1, sizeof(NODE))))
		ERR;
	node->parent = NULL;
	node->isLeaf = false;
	node->key[0] = rs_key;
	node->chi[0] = left;
	node->chi[1] = right;
	node->nkey = 1;

	return node;
}

NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf)
		return node;
	for (kid = 0; kid < node->nkey; kid++)
	{
		if (key < node->key[kid])
			break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
	if (key < leaf->key[0])
	{
		for (i = leaf->nkey; i > 0; i--)
		{
			leaf->chi[i] = leaf->chi[i - 1];
			leaf->key[i] = leaf->key[i - 1];
		}
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
	else
	{
		// Step 2. Insert the new key
		for (i = 0; i < leaf->nkey; i++)
		{
			if (key < leaf->key[i])
			{
				break;
			}
		}
		for (int j = leaf->nkey; j > i; j--)
		{
			leaf->chi[j] = leaf->chi[j - 1];
			leaf->key[j] = leaf->key[j - 1];
		}
	}
	leaf->key[i] = key;
	leaf->chi[i] = (NODE *)data;
	// Step 1. Increment the number of keys
	leaf->nkey++;
	return leaf;
}

void copy_from_left_to_temp(TEMP *temp, NODE *left)
{
	int i;
	// bzero(temp, sizeof(TEMP));
	for (i = 0; i < (N - 1); i++)
	{
		temp->chi[i] = left->chi[i];
		temp->key[i] = left->key[i];
	}
	temp->nkey = N - 1;
	temp->chi[i] = left->chi[i];
}

void insert_in_temp(TEMP *temp, int key, void *data)
{
	int i;
	if (key < temp->key[0])
	{
		for (i = temp->nkey; i > 0; i--)
		{
			temp->chi[i] = temp->chi[i - 1];
			temp->key[i] = temp->key[i - 1];
		}
		temp->key[0] = key;
		temp->chi[0] = (NODE *)data;
	}
	else
	{
		for (i = 0; i < temp->nkey; i++)
		{
			if (key < temp->key[i])
				break;
		}
		for (int j = temp->nkey; j > i; j--)
		{
			temp->chi[j] = temp->chi[j - 1];
			temp->key[j] = temp->key[j - 1];
		}
		temp->key[i] = key;
		temp->chi[i] = (NODE *)data;
	}

	temp->nkey++;
}

void copy_from_temp_to_left_parent(TEMP *temp, NODE *left)
{
	for (int i = 0; i < (int)ceil((N + 1) / 2); i++)
	{
		left->key[i] = temp->key[i];
		left->chi[i] = temp->chi[i];
		left->nkey++;
	}
	left->chi[(int)ceil((N + 1) / 2)] = temp->chi[(int)ceil((N + 1) / 2)];
}

void copy_from_temp_to_right_parent(TEMP *temp, NODE *right)
{
	int id;

	for (id = ((int)ceil((N + 1) / 2) + 1); id < N; id++)
	{
		right->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp->chi[id];
		right->key[id - ((int)ceil((N + 1) / 2) + 1)] = temp->key[id];
		right->nkey++;
	}
	right->chi[id - ((int)ceil((N + 1) / 2) + 1)] = temp->chi[id];

	for (int i = 0; i < right->nkey + 1; i++)
		right->chi[i]->parent = right;
}

void copy_from_temp_to_left(TEMP temp, NODE *left)
{
	for (int i = 0; i < (int)ceil((double)N / (double)2); i++)
	{
		left->key[i] = temp.key[i];
		left->chi[i] = temp.chi[i];
		left->nkey++;
	}
}

void copy_from_temp_to_right(TEMP temp, NODE *right)
{
	for (int i = (int)ceil((double)N / (double)2); i < N; i++)
	{
		right->key[i - (int)ceil((double)N / (double)2)] = temp.key[i];
		right->chi[i - (int)ceil((double)N / (double)2)] = temp.chi[i];
		right->nkey++;
	}
}

void insert_after_left_child(NODE *parent, NODE *left_child, int rs_key, NODE *right_child)
{
	int lcid = 0;
	int rcid = 0;
	int i;

	for (i = 0; i < parent->nkey + 1; i++)
	{
		if (parent->chi[i] == left_child)
		{
			lcid = i;
			rcid = lcid + 1;
			break;
		}
	}
	assert(i != parent->nkey + 1);

	for (i = parent->nkey + 1; i > rcid; i--)
		parent->chi[i] = parent->chi[i - 1];
	for (i = parent->nkey; i > lcid; i--)
		parent->key[i] = parent->key[i - 1];

	parent->key[lcid] = rs_key;
	parent->chi[rcid] = right_child;
	parent->nkey++;
}

void insert_temp_after_left_child(TEMP *temp, NODE *left_child, int rs_key, NODE *right_child)
{
	int lcid = 0;
	int rcid = 0;
	int i;

	for (i = 0; i < temp->nkey + 1; i++)
	{
		if (temp->chi[i] == left_child)
		{
			lcid = i;
			rcid = lcid + 1;
			break;
		}
	}
	assert(i != temp->nkey + 1);

	for (i = temp->nkey + 1; i > rcid; i--)
		temp->chi[i] = temp->chi[i - 1];
	for (i = temp->nkey; i > lcid; i--)
		temp->key[i] = temp->key[i - 1];

	temp->key[lcid] = rs_key;
	temp->chi[rcid] = right_child;
	temp->nkey++;
}

void erase_entries(NODE *node)
{
	for (int i = 0; i < N - 1; i++)
		node->key[i] = 0;
	for (int i = 0; i < N; i++)
		node->chi[i] = NULL;
	node->nkey = 0;
}

void insert_in_parent(NODE *left_child, int rs_key, NODE *right_child)
{
	NODE *left_parent;
	NODE *right_parent;

	if (left_child == Root)
	{
		Root = alloc_root(left_child, rs_key, right_child);
		left_child->parent = right_child->parent = Root;
		return;
	}
	left_parent = left_child->parent;
	if (left_parent->nkey < N - 1)
	{
		insert_after_left_child(left_parent, left_child, rs_key, right_child);
	}
	else
	{ // split
		TEMP temp;
		copy_from_left_to_temp(&temp, left_parent);
		insert_temp_after_left_child(&temp, left_child, rs_key, right_child);

		erase_entries(left_parent);
		right_parent = alloc_internal(left_parent->parent);
		copy_from_temp_to_left_parent(&temp, left_parent);
		int rs_key_parent = temp.key[(int)ceil((double)N / (double)2)];
		copy_from_temp_to_right_parent(&temp, right_parent);
		insert_in_parent(left_parent, rs_key_parent, right_parent);
	}
}

void insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL)
	{
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
	else
	{
		leaf = find_leaf(Root, key);
	}

	if (leaf->nkey < (N - 1))
	{
		insert_in_leaf(leaf, key, data);
	}
	else
	{
		// Split (quiz at 10/09)
		NODE *left = leaf;
		NODE *right = alloc_leaf(leaf->parent);
		TEMP temp;

		copy_from_left_to_temp(&temp, left);
		insert_in_temp(&temp, key, data);
		right->chi[N - 1] = left->chi[N - 1];
		left->chi[N - 1] = right;
		erase_entries(left);
		copy_from_temp_to_left(temp, left);
		copy_from_temp_to_right(temp, right);
		int rs_key = right->key[0];
		insert_in_parent(left, rs_key, right);
	}
}

void init_root(void)
{
	Root = NULL;
}

void search_core(const int key)
{
	NODE *n = find_leaf(Root, key);
	for (int i = 0; i < n->nkey + 1; i++)
	{
		if (n->key[i] == key)
			return;
	}
	cout << "Key not found: " << key << endl;
	ERR;
}

int interactive()
{
	int key;

	std::cout << "Key: ";
	std::cin >> key;

	return key;
}

int main(int argc, char *argv[])
{
	init_root();

	while (true)
	{
		insert(interactive(), NULL);
		print_tree(Root);
	}

	return 0;
}
