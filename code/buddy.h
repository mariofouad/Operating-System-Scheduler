#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Definition of the TreeNode structure
typedef struct TreeNode
{
    int size;
    bool allocated;
    int pid;
    int stIndex;
    int endIndex;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

// Function to initialize memory
TreeNode *initialize_memory(int max_size, int start, int end)
{
    TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));
    root->stIndex = start;
    root->endIndex = end;
    root->size = max_size;
    root->allocated = false;
    root->pid = -1;
    root->left = NULL;
    root->right = NULL;
    return root;
}

// Function to allocate memory
TreeNode *allocate_memory(TreeNode *root, int size, int processid)
{
    if (root == NULL)
    {
        return NULL;
    }

    if (!(root->allocated) && root->size >= size && !(root->left) && !(root->right))
    {
        if (root->size == size)
        {
            root->allocated = true;
            root->pid = processid;
            return root;
        }
        // Split the block if it's larger than needed
        int left_size = root->size / 2;
        root->left = initialize_memory(left_size, root->stIndex, (root->size / 2) - 1 + root->stIndex);
        root->right = initialize_memory(left_size, (root->size / 2) + root->stIndex, root->endIndex);
        return allocate_memory(root->left, size, processid);
    }

    TreeNode *left_alloc = allocate_memory(root->left, size, processid);
    if (left_alloc)
    {
        return left_alloc;
    }
    else
    {
        return allocate_memory(root->right, size, processid);
    }
}

// Function to find and merge buddy blocks
void merge_buddies(TreeNode *root)
{
    if (root == NULL || root->left == NULL || root->right == NULL || root->left->left || root->left->right || root->right->left || root->right->right)
    {
        return;
    }

    if (!(root->left->allocated) && !(root->right->allocated))
    {
        // Both left and right child blocks are free, merge them
        free(root->left);
        free(root->right);
        root->left = NULL;
        root->right = NULL;
    }
}

// Function to deallocate memory
TreeNode *deallocate_memory(TreeNode *root, int NodeId)
{
    if (root == NULL)
    {
        return NULL;
    }

    if (root->left && root->left->pid == NodeId)
    {
        TreeNode *saved = root->left;
        root->left->allocated = false;
        root->left->pid = -1;
        merge_buddies(root);
        return saved;
    }

    if (root->right && root->right->pid == NodeId)
    {
        TreeNode *saved = root->right;
        root->right->allocated = false;
        root->right->pid = -1;
        merge_buddies(root);
        return saved;
    }

    TreeNode *savedLeft = deallocate_memory(root->left, NodeId);
    TreeNode *savedRight = deallocate_memory(root->right, NodeId);
    merge_buddies(root);

    return savedLeft == NULL ? savedRight : savedLeft;
}

// Function to print memory usage
void print_memory_usage(TreeNode *root)
{
    if (root == NULL)
    {
        return;
    }

    print_memory_usage(root->left);
    printf("Block size: %d bytes, with process id: %d, Allocated: %s from i %d to j %d\n", root->size, root->pid, root->allocated ? "Yes" : "No", root->stIndex, root->endIndex);
    print_memory_usage(root->right);
}

// // Example usage
// int main() {
//     int max_memory_size = 1024;
//     TreeNode* memory_tree = initialize_memory(max_memory_size);

//     // Allocate memory blocks of different sizes
//     TreeNode* allocated_block_1 = allocate_memory(memory_tree, 128);
//     TreeNode* allocated_block_2 = allocate_memory(memory_tree, 256);
//     TreeNode* allocated_block_3 = allocate_memory(memory_tree, 256);

//     // Print memory usage
//     printf("Memory usage after allocation:\n");
//     print_memory_usage(memory_tree);

//     // Deallocate memory blocks
//     deallocate_memory(memory_tree, allocated_block_1);
//     //deallocate_memory(memory_tree, allocated_block_2);
//     deallocate_memory(memory_tree, allocated_block_3);

//     // Print memory usage after deallocation
//     printf("\nMemory usage after deallocation:\n");
//     print_memory_usage(memory_tree);

//     // Free memory allocated for the tree
//     free(memory_tree);

//     return 0;
//  }
