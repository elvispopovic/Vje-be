#include <tree.h>

using namespace std;

void makeTree(node *tree, int levels);
void destroyTree(node *tree);

void inorderTraversal(node *tree);
void preorderTraversal(node *tree);
void postorderTraversal(node *tree);
void levelOrderTraversal(node *tree, int levels);

int main()
{
    const int levels = 5;
    node *tree = new node;
    makeTree(tree,levels);
    cout << "Tree created." << endl;

    cout << "Inorder traversal:" << endl;
    inorderTraversal(tree);
    cout << endl;

    cout << "Preorder traversal:" << endl;
    preorderTraversal(tree);
    cout << endl;
    cout << "Postorder traversal:" << endl;
    postorderTraversal(tree);
    cout << endl;

    cout << "Level order traversal:" << endl;
    levelOrderTraversal(tree, levels);
    cout << endl;

    destroyTree(tree);
    cout << "Tree deleted." << endl;
    return 0;
}

void makeChildren(node *tree, int depth, int levels, int **levelCounters)
{
    int i;
    tree->value = (*levelCounters)[levels-depth-1]++;
    if(depth > 0)
    {
        depth--;
        for(i=0; i<2; i++)
        {
            tree->children[i] = new node;
            makeChildren(tree->children[i],depth, levels, levelCounters);
        }
    }
    else
        for(i=0; i<2; i++)
            tree->children[i] = nullptr;
}

void makeTree(node *tree, int levels)
{
    int i = 0, start = 1;
    int *levelCounters = new int[levels];
    for(i=0; i<levels; i++)
    {
        levelCounters[i]=start-1;
        start*=2;
    }
     makeChildren(tree, levels-1, levels, &levelCounters);
}



void destroyTree(node *tree)
{
    for(int i=0; i<2; i++)
        if(tree->children[i]!=nullptr)
            destroyTree(tree->children[i]);
    delete tree;
}

void inorderTraversal(node *tree)
{
    if(tree->children[0]!=nullptr)
        inorderTraversal(tree->children[0]);
    cout << tree->value << " ";
    if(tree->children[1]!=nullptr)
        inorderTraversal(tree->children[1]);
}

void preorderTraversal(node *tree)
{
    cout << tree->value << " ";
    if(tree->children[0]!=nullptr)
        preorderTraversal(tree->children[0]);
    if(tree->children[1]!=nullptr)
        preorderTraversal(tree->children[1]);
}

void postorderTraversal(node *tree)
{
    if(tree->children[0]!=nullptr)
        postorderTraversal(tree->children[0]);
    if(tree->children[1]!=nullptr)
        postorderTraversal(tree->children[1]);
    cout << tree->value << " ";
}

void visitNode(node **fifo, int begin, int end, int size)
{
    if(begin==end)
        return;
    cout << fifo[begin]->value << " ";
    for(int i=0; i<2; i++)
        if(fifo[begin]->children[i]!=nullptr)
        {
            fifo[end++]=(fifo[begin]->children[i]);
            if(end>=size)
                end=0;
        }
    begin++;
    if(begin>=size)
        begin = 0;
    visitNode(fifo,begin,end,size);
}

void levelOrderTraversal(node *tree, int levels)
{
    int i, s, begin, end;
    node **fifo;
    for(i=0, s=1; i<levels-1; i++, s*=2);
    fifo = new node*[s+1];
    fifo[0]=tree;
    begin = 0; 
    end = 1;
    visitNode(fifo, begin, end, s+1);
    delete[] fifo;
}

