#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#define DATATYPE int
#define FORM "%d"

typedef struct BiTree
{
    int balance;              //平衡因子
    DATATYPE data;            //节点数据
    struct BiTree *left;
    struct BiTree *right;
    struct BiTree *father;    //指向双亲节点
} AVL;

int Depth(AVL *root)
{
    //计算树的深度。
    int i,j;

    if(root->left)
        i=Depth(root->left);
    else
        i=0;

    if(root->right)
        j=Depth(root->right);
    else
        j=0;

    return i>j?i+1:j+1;
}

void updateBF(AVL *NewNode)
{
    //更新平衡因子
    if(!NewNode || !NewNode->father) return;
    AVL *CurrNode=NewNode;
    AVL *PreNode=CurrNode->father;

    if(CurrNode==PreNode->left)
    {
        //指针从左路回溯
        PreNode->balance++;
        if(PreNode->balance==2) return;
        if(PreNode->left && PreNode->right)   //遇到有两个孩子的节点
            if(PreNode->balance<=0)
                return;
    }
    else
    {
        //指针从右路回溯
        PreNode->balance--;
        if(PreNode->balance==-2) return;
        if(PreNode->left && PreNode->right)
            if(PreNode->balance>=0)
                return;
    }
    updateBF(PreNode);

    return;
}

void RR_Rotate(AVL **root,AVL *centreNode)
{
    //向右旋转,RR型。
    AVL *temp=NULL;

    if(centreNode->right)
    {
        //中心节点的右节点不为空
        centreNode->father->left=centreNode->right;     //将中心节点的右子树连接到双亲节点的左子树上。
        centreNode->right->father=centreNode->father;
    }
    else
        //中心节点的右节点为空
        centreNode->father->left=NULL;

    temp=centreNode->father->father;
    if(centreNode->father->father!=NULL)
        if(centreNode->father->father->left==centreNode->father)
            centreNode->father->father->left=centreNode;
        else
            centreNode->father->father->right=centreNode;
    else
        *root=centreNode;

    centreNode->right=centreNode->father;         //当前节点的双亲成为其右子树。
    centreNode->father->father=centreNode;
    centreNode->father->balance-=2;
    centreNode->father=temp;
    centreNode->balance--;

    return;
}

void LL_Rotate(AVL **root,AVL *centreNode)
{
    //向左旋转,LL型
    AVL *temp=NULL;

    if(centreNode->left)
    {
        //中心节点的左节点不为空
        centreNode->father->right=centreNode->left;    //将中心节点的左子树连接到双亲的右子树上。
        centreNode->left->father=centreNode->father;   //中心节点的左子树的双亲改为中心节点的双亲。
    }
    else
        centreNode->father->right=NULL;

    temp=centreNode->father->father;
    if(centreNode->father->father!=NULL)
        if(centreNode->father->father->left==centreNode->father)
            centreNode->father->father->left=centreNode;
        else
            centreNode->father->father->right=centreNode;
    else
        *root=centreNode;

    centreNode->left=centreNode->father;         //将中心节点的左孩子改为中心节点的双亲
    centreNode->left->father=centreNode;         //将中心节点左孩子的双亲更改为中心节点。
    centreNode->balance++;
    centreNode->father->balance+=2;
    centreNode->father=temp;

    return;
}

void RL_Rotate(AVL **root,AVL *centreNode)
{
    //先右后左旋转
    AVL *temp=NULL;

    temp=centreNode->father;
    centreNode->father->right=centreNode->left;
    centreNode->left->father=centreNode->father;
    centreNode->left->balance--;
    centreNode->balance--;

    if(centreNode->left->right)
    {
        //中心节点的左节点的右孩子不为空。
        temp->right->right->father=centreNode;
    }
    centreNode->left=temp->right->right;
    temp->right->right=centreNode;
    centreNode->father=temp->right;
    if(temp->right->balance==-2)
        temp->right->balance++;
    LL_Rotate(root,temp->right);

    return;
}

void LR_Rotate(AVL **root,AVL *centreNode)
{
    //先左后右旋转，LR型。
    AVL *temp=NULL;

    temp=centreNode->father;
    centreNode->father->left=centreNode->right;
    centreNode->right->father=centreNode->father;
    centreNode->right->balance++;
    centreNode->balance++;

    if(centreNode->right->left)
    {
        //中心节点的左节点的右孩子不为空。
        temp->left->left->father=centreNode;
    }
    centreNode->right=temp->left->left;
    temp->left->left=centreNode;
    centreNode->father=temp->left;
    if(temp->left->balance==2)
        temp->left->balance--;
    RR_Rotate(root,temp->left);

    return;
}

void adjustAVL(AVL **root,AVL *NewNode)
{
    if(!NewNode) return;
    AVL *CurrNode=NewNode;
    AVL *PreNode=NULL;

    while(CurrNode->father!=NULL && CurrNode->father->balance!=2 && CurrNode->father->balance!=-2)
    {
        PreNode=CurrNode;
        CurrNode=CurrNode->father;
    }
    if(!CurrNode->father) return;
    //printf(":%d\n",CurrNode->data);

    if(CurrNode->father->left==CurrNode)
    {
        //自己是双亲的左节点
        if(CurrNode->left==PreNode)
            //指针从左路回溯。
            RR_Rotate(root,CurrNode);
        else
            //指针从右路回溯
            LR_Rotate(root,CurrNode);
    }
    else
    {
        //自己是双亲的右节点
        if(CurrNode->right==PreNode)
            //指针从右路回溯
            LL_Rotate(root,CurrNode);
        else
            //指针从左路回溯
            RL_Rotate(root,CurrNode);
    }

    return;
}

int InsertNode(AVL **root,DATATYPE elem)
{
    AVL *CurrNode=NULL;
    AVL *PreNode=NULL;
    AVL *NewNode=NULL;

    if(*root==NULL)  //如果根节点不存在则创建
    {
        if((*root=(AVL *)malloc(sizeof(AVL)))==NULL)
            return 0;
        memset(*root,NULL,sizeof(AVL));
        (*root)->data=elem;
        return 1;
    }

    CurrNode=*root;
    while(CurrNode)
    {
        PreNode=CurrNode;
        if(elem<CurrNode->data)
            CurrNode=CurrNode->left;
        else if(elem>CurrNode->data)
            CurrNode=CurrNode->right;
        else
            return 2;    //表示已存在该元素
    }
    if((NewNode=(AVL *)malloc(sizeof(AVL)))==NULL)
        return 0;
    memset(NewNode,NULL,sizeof(AVL));
    NewNode->data=elem;        //为新节点赋值
    NewNode->father=PreNode;
    //插入新节点
    if(elem<PreNode->data)
        PreNode->left=NewNode;
    else
        PreNode->right=NewNode;
    updateBF(NewNode);     //更新AVL的平衡因子
    adjustAVL(root,NewNode);    //AVL自调节

    return 1;
}

int InOrderTraversal(AVL *root)
{
    int n=0;
    if(root==NULL)
        return 0;
    n=InOrderTraversal(root->left);
    n++;
    n+=InOrderTraversal(root->right);
    return n;
}

int search(AVL *root,DATATYPE elem)
{
    if(!root) return 0;
    if(elem==root->data) return 1;
    if(elem<root->data)
        if(search(root->left,elem)) return 1;
    if(elem>root->data)
        if(search(root->right,elem)) return 1;
    return 0;
}

int main(int argc,char *argv[])
{
    int j=0,i=0;
    DATATYPE elem;
    AVL *root=NULL;

    while(i<30000)
    {
        srand(j++);
        elem=rand();
        if(InsertNode(&root,elem)==2) continue;    //插入节点。
        printf("已插入%d个节点。\r",++i);
    }
    //InOrderTraversal(root);
    puts("");
    printf("树深:%d\n\n\n",Depth(root));

    while(1)
    {
        printf("有%d个节点。\n",InOrderTraversal(root));
        printf("左子树深度:%d,右子树深度:%d\n",Depth(root->left),Depth(root->right));
        printf("输入查找的元素:");
        fflush(stdin);
        if(scanf(FORM,&elem)!=1) continue;
        if(search(root,elem))
            printf("已找到。\n");
        else
        {
            printf("没找到，已添加。\n");
            InsertNode(&root,elem);
        }
    }


    return 0;
}












