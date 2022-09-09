#include<iostream>
#include<tree.h>
using namespace std;
pNodoA* InsereArvore(pNodoA *a, int ch)
{
     if (a == NULL)
     {
         a =  (pNodoA*) malloc(sizeof(pNodoA));
         cout << ch << endl;
         a->info = ch;
         a->esq = NULL;
         a->dir = NULL;
         a->currX = -800;
         a->currY = -800;
         return a;
     }
     else
          if (ch < a->info)
              a->esq = InsereArvore(a->esq,ch);
          else if (ch > a->info)
              a->dir = InsereArvore(a->dir,ch);
     return a;
}


pNodoA* consultaABP(pNodoA *a, int chave) {

    while (a!=NULL){
          if (a->info == chave )
             return a; 
          else
            if (a->info > chave)
               a = a->esq;
            else
               a = a->dir;
            }
            return NULL; 
}

pNodoA* minValor(pNodoA* node)
{
    struct TNodoA* current = node;
  
    /* loop down to find the leftmost leaf */
    while (current && current->esq != NULL)
        current = current->esq;
  
    return current;
}
  

pNodoA* RemoveArvore(pNodoA *a, int ch)
{
      // base case
    if (a == NULL)
        return a;
  
    // If the ch to be deleted is 
    // smaller than the a's
    // ch, then it lies in esq subtree
    if (ch < a->info)
        a->esq = RemoveArvore(a->esq, ch);
  
    // If the ch to be deleted is
    // greater than the a's
    // ch, then it lies in dir subtree
    else if (ch > a->info)
        a->dir = RemoveArvore(a->dir, ch);
  
    // if ch is same as a's ch, then This is the node
    // to be deleted
    else {
        // node has no child
        if (a->esq==NULL and a->dir==NULL)
            return NULL;
        
        // node with only one child or no child
        else if (a->esq == NULL) {
            struct TNodoA* temp = a->dir;
            free(a);
            return temp;
        }
        else if (a->dir == NULL) {
            struct TNodoA* temp = a->esq;
            free(a);
            return temp;
        }
  
        // node with two children: Get the inorder successor
        // (smallest in the dir subtree)
        struct TNodoA* temp = minValor(a->dir);
  
        // Copy the inorder successor's content to this node
        a->info = temp->info;
  
        // Delete the inorder successor
        a->dir = RemoveArvore(a->dir, temp->info);
    }
    return a;
}

int getLevel(pNodoA* currNode) {
	if (currNode == NULL)
		return 0;
	return max(getLevel(currNode->esq) + 1, getLevel(currNode->dir) + 1);
}