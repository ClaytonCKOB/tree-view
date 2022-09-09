#include<iostream>
#include<math.h>
struct TNodoA{
        int info;
        double x;
	    double y;
        double currX = 0;
	    double currY = 0;
        bool emPosicao = false;
        struct TNodoA *esq;
        struct TNodoA *dir;
        int level;
        int col;

};
typedef struct TNodoA pNodoA;

pNodoA* InsereArvore(pNodoA *a, int ch);


pNodoA* consultaABP(pNodoA *a, int chave);
pNodoA* minValor(pNodoA* node);
  

pNodoA* RemoveArvore(pNodoA *a, int ch);

int getLevel(pNodoA* currNode);