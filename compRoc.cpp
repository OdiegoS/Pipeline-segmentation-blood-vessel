#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/highgui/highgui_c.h>

#include <opencv2/imgproc.hpp>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**********/
FILE *lista;


typedef struct Roc{
	// igs[0] = verdadeiro negativo (TN). igs[1] = verdadeiro positivo (TP)	
	float igs[2];
	// dif[0] = falso negativo (FN). dif[1] = falso positivo (FP)
	float dif[2];
}curvaR;

typedef struct estrutRoc{
	curvaR *cR = NULL;
}eR;


void taxa(float igs[2], float dif[2], float val[]){
	val[0] = ( igs[1]/(igs[1]+dif[0]) );
	val[1] = ( igs[0]/(igs[0]+dif[1]) );
	val[2] = ( dif[1]/(igs[0]+dif[1]) );
	val[3] = ( dif[0]/(igs[1]+dif[0]) );
	val[4] = ( igs[1]/(igs[1]+dif[1]) );
	val[5] = ( igs[0]/(igs[0]+dif[0]) );
	val[6] = ( (igs[1]+igs[0])/(igs[0]+igs[1]+dif[0]+dif[1]) );
	
	//printf("Val 0: %f\nVal 1: %f\nVal 2: %f\nVal 3: %f\n", val[0], val[1], val[2], val[3]);
	
}
// 0 preto, -1 n-preto
void curvasRoc(IplImage *imgMet,IplImage *imgDb, int tam, curvaR *cR){
	int i;

	for(i=0;i<2;i++){
		cR->igs[i] = 0;
		cR->dif[i] = 0;
	}

	//Binarizando as imagens
	IplImage *tmp = cvCreateImage( cvGetSize(imgMet), imgMet->depth, imgMet->nChannels);
	//opcional
	cvThreshold(imgMet, tmp, 0, 255, CV_THRESH_BINARY);	
	cvCopy(tmp,imgMet);

	cvThreshold(imgDb, tmp, 3, 255, CV_THRESH_BINARY);
	cvCopy(tmp,imgDb);


	//Calculando os valores de TN, TP, FN e FP
	for(i=0;i<tam;i++){
	    // Se forem iguais: podem ser TN ou TP
	    if(imgMet->imageData[i] == imgDb->imageData[i]){
		// Se forem iguais com valor 0, então é TN
		if(imgMet->imageData[i] == 0){
			cR->igs[0]++;
		// Se forem iguais com valor acima de 0, então é TP
		}else{
			cR->igs[1]++;
		}
	    // Se forem diferentes: podem ser FN ou FP
	    }else{
		// Se forem diferentes e imagem obtida for 0, então é um FN
		if(imgMet->imageData[i] == 0){
			cR->dif[0]++;
		// Se forem diferentes e imagem obtida for acima de 0, então é um FP
		}else{
			cR->dif[1]++;
		}
	    }
 	}

//	printf("Tam: %d\nVN: %f  VP: %f\nFN: %f   FP: %f\n", tam, cR->igs[0], cR->igs[1], cR->dif[0], cR->dif[1]);
}

void salvarFile(FILE *saida, int numImg, int diretorio, eR *comp){
	int i,j,k;
	float val[7] = { 0, 0, 0, 0, 0, 0, 0};

	for(i=1;i<=diretorio;i++){
		if(i==1){
			fprintf(saida, "        |                                Referencia 1                                |");
		}else{
			fprintf(saida, "|                                Referencia %d                                |", i);
		}
	}
	fprintf(saida,"\n Imagem |");
  	for(i=1;i<=diretorio;i++){
		if(i!=diretorio){
			fprintf(saida, "  SN(TPR) |  SP(TNR) |   FPR    |   FNR    |   PPV    |   NPV    |   ACC    ||");
		}else{
			fprintf(saida, "  SN(TPR) |  SP(TNR) |   FPR    |   FNR    |   PPV    |   NPV    |   ACC    |");
		}
  	}

	for(i=0;i<numImg;i++){
		if(i<9){
			fprintf(saida, "\n   0%d   |", i+1);
		}else{
			fprintf(saida, "\n   %d   |", i+1);
		}

		for(j=0;j<diretorio;j++){
			taxa(comp[j].cR[i].igs, comp[j].cR[i].dif, val);
			k = 0;			
			while(k != 7){
				if(val[k] < 0.100){
					fprintf(saida,"  0%.2f%c  |", val[k]*100, 37);
				}else{
					fprintf(saida,"  %.2f%c  |", val[k]*100, 37);
				}
				k++;
			}
			if(j != (diretorio-1) ){
				fprintf(saida, "|");
			}
		}	
	}


	for(i=0;i<diretorio;i++){
		if(i==0){
			fprintf(saida,"\n-------------------------------------------------------------------------------------|");
		}else{
			fprintf(saida,"|----------------------------------------------------------------------------|");
		}
	}

	fprintf(saida,"\n Tot(%c) |", 37);

	curvaR *suporte = (curvaR*)malloc( diretorio * sizeof(curvaR) );
	for(i=0;i<diretorio;i++){
		suporte[i].igs[0] = 0;
		suporte[i].igs[1] = 0;
		suporte[i].dif[0] = 0;
		suporte[i].dif[1] = 0;
	}
	
	for(i=0;i<diretorio;i++){
		for(j=1;j<numImg;j++){ 
			suporte[i].igs[0] = suporte[i].igs[0] + comp[i].cR[j].igs[0];
			suporte[i].igs[1] = suporte[i].igs[1] + comp[i].cR[j].igs[1];
			suporte[i].dif[0] = suporte[i].dif[0] + comp[i].cR[j].dif[0];
			suporte[i].dif[1] = suporte[i].dif[1] + comp[i].cR[j].dif[1];
		}
		taxa(suporte[i].igs, suporte[i].dif, val);
		k = 0;			
		while(k != 7){
			if(val[k] < 0.100){
				fprintf(saida,"  0%.2f%c  |", val[k]*100, 37);
			}else{
				fprintf(saida,"  %.2f%c  |", val[k]*100, 37);
			}
			k++;
		}
		if(i != (diretorio-1) ){
			fprintf(saida, "|");
		}
	}
	for(i=1;i<diretorio;i++){
		suporte[0].igs[0] = suporte[0].igs[0] + suporte[i].igs[0];
		suporte[0].igs[1] = suporte[0].igs[1] + suporte[i].igs[1];
		suporte[0].dif[0] = suporte[0].dif[0] + suporte[i].dif[0];
		suporte[0].dif[1] = suporte[0].dif[1] + suporte[i].dif[1];
	}
	taxa(suporte[0].igs, suporte[0].dif, val);
	fprintf(saida, "\n\n\n\n           |  SN(TPR) |  SP(TNR) |   FPR    |   FNR    |   PPV    |   NPV    |   ACC    |\n");
/*### */printf("\n  SN(TPR) |  SP(TNR) |   FPR    |   FNR    |   PPV    |   NPV    |   ACC    |\n");
	fprintf(saida," Geral(%c)  |", 37);
	k=0;	
	while(k != 7){
		if(val[k] < 0.100){
			fprintf(saida,"  0%.2f%c  |", val[k]*100, 37);
/* ########## */	printf("  0%.2f%c  |", val[k]*100, 37);
		}else{
			fprintf(saida,"  %.2f%c  |", val[k]*100, 37);
/* ########## */	printf("  %.2f%c  |", val[k]*100, 37);
		}
		k++;

	}

/**********/
	fprintf(lista,"TPR: %.2f%c, FPR: %.2f%c, ACC: %.2f%c\n",val[0]*100, 37, val[2]*100,37, val[6]*100,37);
/*##*/ printf("\n\n");
	fprintf(saida, "\n\n\n\n");
	for(i=1;i<=diretorio;i++){
		if(i==1){
			fprintf(saida, "                                   Referencia 1                        |");
		}else{
			fprintf(saida, "|                         Referencia %d                         |", i);
		}
	}
	fprintf(saida,"\n Imagem |");
  	for(i=1;i<=diretorio;i++){
		if(i!=diretorio){
			fprintf(saida, "   TP   |   TN   |   FP   |   FN   |  TP+FP |  TP+FN | Total  ||");
		}else{
			fprintf(saida, "   TP   |   TN   |   FP   |   FN   |  TP+FP |  TP+FN | Total  |");
		}
  	}


	for(i=0;i<numImg;i++){
		if(i<9){
			fprintf(saida, "\n   0%d   |", i+1);
		}else{
			fprintf(saida, "\n   %d   |", i+1);
		}

		for(j=0;j<diretorio;j++){
		  fprintf(saida," %6.f | %6.f |", comp[j].cR[i].igs[1], comp[j].cR[i].igs[0]);
		  fprintf(saida," %6.f | %6.f |", comp[j].cR[i].dif[1], comp[j].cR[i].dif[0]);
		  fprintf(saida," %6.f |", comp[j].cR[i].igs[1] + comp[j].cR[i].dif[1]);
		  fprintf(saida," %6.f |", comp[j].cR[i].igs[1] + comp[j].cR[i].dif[0]);
		  fprintf(saida," %6.f |", comp[j].cR[i].igs[1] + comp[j].cR[i].igs[0] + comp[j].cR[i].dif[0] + comp[j].cR[i].dif[1]);
		  
		  if(j != (diretorio-1) ){
		  	fprintf(saida, "|");
		  }
		}	
	}
	fprintf(saida,"\n\n\n");
}

void processos(FILE *entrada, FILE *saida){
  int numImg, diretorios,i,tam,j;
  char diret[255];
  IplImage *imgDb = NULL, *imgMet = NULL;
  eR *comp = NULL;

  fscanf(entrada,"%d", &diretorios);
  fscanf(entrada,"%d", &numImg);

  comp = (eR*)malloc(diretorios*sizeof(eR));
  for(i=0;i<diretorios;i++){
	comp[i].cR = (curvaR*)malloc(numImg*sizeof(curvaR));
  }

  //Iniciando o processo de comparacao
  for(i=0;i<numImg;i++){

    fscanf(entrada,"%s",diret);
    imgMet = cvLoadImage(diret, CV_LOAD_IMAGE_GRAYSCALE);

    if( imgMet == NULL){
      printf("*** [Erro] Imagem gerada nao encontrada\n");
      exit(1);
    }

    if( i == 0 ){
      tam = imgMet->height * imgMet->width;
    }

    for(j=0;j<diretorios;j++){
      fscanf(entrada,"%s",diret);
      imgDb = cvLoadImage(diret, CV_LOAD_IMAGE_GRAYSCALE);

      if( imgDb == NULL){
        printf("*** [Erro] Imagem %d da referencia %d nao encontrada\n", i+1,j+1);
        exit(1);
      }

      curvasRoc(imgMet, imgDb, tam, &comp[j].cR[i]);
      cvReleaseImage(&imgDb);
    }

    cvReleaseImage(&imgMet);
  }

  salvarFile(saida,numImg,diretorios,comp);

  for(i=0;i<diretorios;i++){
	  free(comp[i].cR);
	  comp[i].cR = NULL;
  }
  free(comp);
  comp = NULL;

}


int main (int argc, char** argv){

  FILE *entrada = fopen(argv[1],"r");
  FILE *saida = fopen("Resultados/ResultDrive_01-40.txt","w");

/*****/ lista = fopen(argv[8], "a");

  if( entrada == NULL ){
      printf("*** [Erro] Nao foi possivel abrir o arquivo de entrada\n");
      exit(1);
  }

  if( saida == NULL ){
      printf("*** [Erro] Nao foi possivel abrir o arquivo de saida\n");
      exit(1);
  }

/*****/
  fprintf(lista,"Metodo %d -> Tamanho %d, Jan: %dx%d, Val(x,y): %dx%d, BH: %d, TS: %d,  ", atoi(argv[2]), atoi(argv[3]), atoi(argv[5]), atoi(argv[5]), atoi(argv[6]), atoi(argv[6]), atoi(argv[4]), atoi(argv[7]) );
  processos(entrada, saida);
/****/
  fflush(lista);
  fclose(lista);

  fflush(saida);
  fclose(entrada);
  fclose(saida);
  return 0;
}
