#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui/highgui_c.h>
#define __OPENCL__
#include <visiongl.h>

int jan1 = 3;
int jan2 = 3;
int val1 = 1;
int val2 = 3;
float tsy = 3.0;
int b = 1.0;

void vglMin(VglImage *input_1, VglImage *input_2, VglImage *output){

  vglCheckContext(output, VGL_RAM_CONTEXT);
  vglCheckContext(output, VGL_RAM_CONTEXT);

  cvMin(input_1->ipl, input_2->ipl, output->ipl );

  vglSetContext(output, VGL_RAM_CONTEXT);
  
}

void geodesicDilation(VglImage *mask, VglImage *marker, VglImage *temp, float element[],int x,int y, int type){

  VglImage *imagemDilat = vglCreateImage(mask);

  if(type > 0){
	vglClFuzzyDilate(marker, imagemDilat,element,x,y,type);
  }else{
  	vglClDilate(marker,imagemDilat,element,x,y);
  }
  vglClDownload(imagemDilat);

  vglMin(mask,imagemDilat,temp);

  vglReleaseImage(&imagemDilat);
}

void reconstructionByDilation(VglImage *imagem, VglImage *marker, VglImage *temp, float element[],int x,int y,int type){

  geodesicDilation(imagem, marker, temp, element,x,y, type);

   int k=0, tam = imagem->shape[0]*imagem->shape[1];
   for(int i = 0; i < tam; i++){
     if( (marker->ipl->imageData[i] - temp->ipl->imageData[i]) != 0){
      k = 1;
      break;
    }
   }
   if(k == 1){
     reconstructionByDilation(imagem, temp, marker, element,x,y, type);
   }

}

void openingByReconstruction(VglImage* src, VglImage* dst,VglImage *bufferEroded,VglImage *bufferTmp, float element[],int x,int y, int iterations, int type){

  int i;
  
  if(iterations > 0){

    if(type > 0){
    	vglClFuzzyErode(src, bufferTmp,element,x,y,type);
    }else{
    	vglClErode(src,bufferTmp,element,x,y);
    }  
 
    for(int i = 1; i < iterations; i++){
      if (i % 2 == 0){
	if(type > 0){
		vglClFuzzyErode(bufferEroded, bufferTmp,element,x,y,type);
	}else{
		vglClErode(bufferEroded, bufferTmp,element,x,y);
	}
      }else{
	if(type > 0){
		vglClFuzzyErode(bufferTmp, bufferEroded,element,x,y,type);
	}else{
		vglClErode(bufferTmp,bufferEroded,element,x,y);
	}
      }
    }
    if (iterations % 2 == 1){
       vglClCopy(bufferTmp,bufferEroded);
    }
   }
  vglClDownload(bufferEroded);

  reconstructionByDilation(src, bufferEroded, bufferTmp, element,x,y, type);
  
  vglClCopy(bufferTmp,dst);
  vglClDownload(dst);

}


void closingImage(VglImage *src, VglImage *dst,float element[],int x, int y, int iterations, int type){

  int i;
  VglImage *tmp = vglCreateImage(src);

  if(iterations > 0){

    if(type > 0){
    	vglClFuzzyDilate(src,tmp,element,x,y,type);
    }else{
        vglClDilate(src,tmp,element,x,y); 
    }
    for(int i = 1; i < iterations; i++){
      if (i % 2 == 0){
	if(type > 0){
		vglClFuzzyDilate(dst,tmp,element,x,y,type);
	}else{
		vglClDilate(dst,tmp,element,x,y);
	}
      }else{
	if(type > 0){
		vglClFuzzyDilate(tmp, dst,element,x,y,type);
	}else{
		vglClDilate(tmp,dst,element,x,y);
	}
      }
    }
    if (iterations % 2 == 1){
      vglClCopy(tmp,dst);
    }

    if(type > 0){
    	vglClFuzzyErode(dst,tmp,element,x,y,type);
    }else{
    	vglClErode(dst,tmp,element,x,y);  
    }

    for(int i = 1; i < iterations; i++){
      if (i % 2 == 0){
	if(type > 0){
		vglClFuzzyErode(dst,tmp,element,x,y,type);
	}else{
		vglClErode(dst,tmp,element,x,y);
	}
      }else{
	if(type > 0){
		vglClFuzzyErode(tmp, dst,element,x,y,type);
	}else{
		vglClErode(tmp,dst,element,x,y);
	}
      }
    }
    if (iterations % 2 == 1){
      vglClCopy(tmp,dst);
      }
  }
  vglClDownload(dst);

  vglReleaseImage(&tmp);

}

void blackHat (VglImage *src, VglImage *dst,float element[], int x, int y, int interations, int type){

  VglImage *tmp = vglCreateImage(src);
  VglImage *tmp2 = vglCreateImage(src);
	
  closingImage(src, tmp, element, x, y, interations, type);
	
  vglClSub(tmp, src, tmp2);
  vglClCopy(tmp2, dst);
  vglClDownload(dst);

  vglReleaseImage(&tmp);
  vglReleaseImage(&tmp2);

}

void gaussianBlurImage (IplImage *src, IplImage *dst, int Kwidth, int Kheight, int x, int y = 0){

  using namespace cv;

  Mat temp = cvarrToMat(src);
  GaussianBlur(temp, temp, Size(Kwidth,Kheight),x,y);
  *dst = temp;
}

float* gaussianFunction2D(int sizeX, int sizeY, float sigma){
	int i, centro, limX, limY, x, y;
	limX = (sizeX - 1) / 2;
	limY = (sizeY - 1) / 2;
        sigma = limX / sigma;
	float t = 2*( pow(sigma,2) );
	float m = 1 / (M_PI*t);
	float fator, *element = NULL;
	
	if ( (sizeX % 2 == 0) || (sizeY % 2 == 0) ){
		printf("[Error] Valor de X e Y nao podem ser par\n");
		exit(1);
	}
	
	if (sigma <= 0){
		printf("[Error] Valor de sigma deve ser positivo nao nulo\n");
		exit(1);
	}
	

	element = (float*)malloc( (sizeX*sizeY)*sizeof(float) );
	
	centro = ( (sizeX*sizeY) - 1) / 2;
	
	//Calculando o valor para o centro e calculando o fator de normalização
	element[centro] = m * ( pow(M_E, 0) );
	fator = 1 / element[centro];
	element[centro] = 1;
	
	//Calculando os valores das demais coordenadas utilizando no final o fator de normalização
	x = -limX;
	y = -limY;
	
	for(i=0; i<(sizeX*sizeY); i++){
		
		 if(i != centro ){
		 	
		 	float n = ( -( pow(x,2) + pow(y,2) ) ) / t;
		 	element[i] = (m * ( pow(M_E, n)) )*fator;
		 	
		 }
		 y++;
		 
		 if( y == (limY+1) ){
		 	y = - limY;
		 	x++;
		 }
	}
	
	return element;
};

float* gaussianFunction2D2(int sizeX, int sizeY, float sigma){
	int i, centro, limX, limY, x, y;
	limX = (sizeX - 1) / 2;
	limY = (sizeY - 1) / 2;
	float t = 2*( pow(sigma,2) );
	float m = 1 / (M_PI*t);
	float fator, *element = NULL;
	
	if ( (sizeX % 2 == 0) || (sizeY % 2 == 0) ){
		printf("[Error] Valor de X e Y nao podem ser par\n");
		exit(1);
	}
	
	if (sigma <= 0){
		printf("[Error] Valor de sigma deve ser positivo nao nulo\n");
		exit(1);
	}
	

	element = (float*)malloc( (sizeX*sizeY)*sizeof(float) );
	
	centro = ( (sizeX*sizeY) - 1) / 2;
	
	//Calculando o valor para o centro e calculando o fator de normalização
	element[centro] = m * ( pow(M_E, 0) );
	fator = 1 / element[centro];
	element[centro] = 1;
	
	//Calculando os valores das demais coordenadas utilizando no final o fator de normalização
	x = -limX;
	y = -limY;
	
	for(i=0; i<(sizeX*sizeY); i++){
		
		 if(i != centro ){
		 	
		 	float n = ( -( pow(x,2) + pow(y,2) ) ) / t;
		 	element[i] = (m * ( pow(M_E, n)) )*fator;
		 	
		 }
		 y++;
		 
		 if( y == (limY+1) ){
		 	y = - limY;
		 	x++;
		 }
	}
	
	return element;
};


void aplicarMask(IplImage *src, IplImage *mask, IplImage *dst){
  IplImage *tmp = cvCloneImage(mask);

  cvMin(src,mask,tmp);
  cvCopy(tmp,dst);

  cvReleaseImage(&tmp);
}

void metodo(FILE *entrada,int nImg,int type,int tam, int bh){
  int i;
  char diretorio[200];


  vglClInit();
  vglInit();

  for(i = 0; i<nImg;i++){
    if(i<9){
    	printf("Image: 0%d\n", i+1);
    }else{
    	printf("Image: %d\n",i+1);
    }
  
    fscanf(entrada,"%s", diretorio);
 
//    VglImage *vImage = vglLoadImage(diretorio);
    IplImage *suporte = cvLoadImage(diretorio);
    VglImage *vImage = vglCopyCreateImage (suporte);
    cvReleaseImage(&suporte);
    IplImage *tmp = cvCreateImage(cvGetSize(vImage->ipl),vImage->ipl->depth, 1);


    float *element = gaussianFunction2D( tam, tam, ( (tam - 1) / 2) * (1.0/(5.0*tam)));   
    //float *element = gaussianFunction2D2( tam, tam, ( (tam) / x ) );

    cvSplit(vImage->ipl, NULL, tmp, NULL, NULL);
    vglReleaseImage(&vImage);

    IplImage *mask = cvCreateImage(cvGetSize(tmp),tmp->depth, tmp->nChannels);
    cvThreshold(tmp,mask, 20, 255, CV_THRESH_BINARY);
    aplicarMask(tmp,mask,tmp);

    IplImage *imagem = cvCreateImage(cvGetSize(tmp), tmp->depth, 3);
    cvMerge(tmp,tmp,tmp,NULL, imagem);
    cvReleaseImage(&tmp);
    gaussianBlurImage(imagem,imagem, jan1, jan2, val1, val2); 

    vImage = vglCopyCreateImage( imagem );   
    vglImage3To4Channels(vImage);

    blackHat( vImage, vImage, element, tam, tam, bh, type);
//---------------------------
    vglImage4To3Channels(vImage);

    IplImage *mask2 = cvCreateImage(cvGetSize(imagem),imagem->depth, imagem->nChannels);
    cvMerge(mask,mask,mask,NULL, mask2);
    cvCopy(vImage->ipl, imagem);
    cvErode(mask2,mask2, NULL, 10);
    aplicarMask(imagem,mask2, imagem);

    vImage = vglCopyCreateImage( imagem );
    cvReleaseImage(&mask);
    cvReleaseImage(&mask2);
    cvReleaseImage(&imagem);

    vglImage3To4Channels(vImage);
    VglImage *buffer0 = vglCreateImage(vImage);
    vglClThreshold(vImage, buffer0, tsy/255.0, 1.0);
    vglClCopy(buffer0,vImage);
    vglClDownload(vImage);

    VglImage *buffer1 = vglCreateImage(vImage);
    VglImage *buffer2 = vglCreateImage(vImage);
    openingByReconstruction( vImage, vImage, buffer1, buffer2, element, tam, tam, 1, type);
    vglReleaseImage(&buffer1); vglReleaseImage(&buffer2);


    vglClThreshold(vImage, buffer0, b/255.0, 1.0);
    vglClCopy(buffer0,vImage);

    vglReleaseImage(&buffer0);
    vglClDownload(vImage);

    vglImage4To3Channels(vImage);

    fscanf(entrada, "%s", diretorio);
    cvSaveImage(diretorio, vImage->ipl);

    vglReleaseImage(&vImage);
  }
}

int main(int argc, char** argv){
  
  FILE *entrada = fopen(argv[2], "r");
  int n;
  int ident;

  fscanf(entrada,"%d", &n);
switch ( atoi(argv[1]) ){
      case 1:
         printf("\n[ Metodo 1 ] - STANDARD\n");
	 ident = VGL_CL_FUZZY_STANDARD;
 	 break;
      case 2:
    	 printf("\n[ Metodo 2 ] - ALGEBRAIC\n");
	 ident = VGL_CL_FUZZY_ALGEBRAIC;
    	 break;
      case 3:
    	 printf("\n[ Metodo 3 ] - BOUNDED\n");
	 ident = VGL_CL_FUZZY_BOUNDED;
    	 break;
      case 4:
	 printf("\n[ Metodo 4 ] - DRASTIC\n");
	 ident = VGL_CL_FUZZY_DRASTIC;
    	 break;
      case 5:
	 printf("\n[ Metodo 5 ] - DUBOIS and PRADE\n");
	 ident = VGL_CL_FUZZY_DAP;
    	 break;
      case 6:
	 printf("\n[ Metodo 6 ] - HAMACHER\n");
	 ident = VGL_CL_FUZZY_HAMACHER;
    	 break;
      case 7:
    	 printf("\n[ Metodo 7 ] - GEOMETRIC\n");
	 ident = VGL_CL_FUZZY_GEOMETRIC;
    	 break;
      case 8:
    	 printf("\n[ Metodo 8 ] - ARITHMETIC\n");
	 ident = VGL_CL_FUZZY_ARITHMETIC;
    	 break;
      case 9:
	 printf("\n[ Metodo 9 ] - CLASSIC\n");
	 ident = 0;
    	 break;
      default:
    	 printf("*** [Erro] Metodo nao criado\n");
    	 exit(1);
  }
//  printf(" *** Tamanho %d, Black Hat: %d, Ts: %f ***\n", atoi(argv[3]), atoi(argv[4]), atof(argv[5]));

  jan1 = atoi(argv[5]);
  jan2 = jan1;
  val1 = atoi(argv[6]);
  val2 = val1;
  tsy = atof(argv[7]);

  printf(" ***Tamanho %d, Jan: %dx%d, Val(x,y): %dx%d, TS: %f ***\n", atoi(argv[3]), jan1, jan1, val1, val1, tsy );

  metodo(entrada, n, ident, atoi(argv[3]), atoi(argv[4]));
  printf("Fim do metodo\n");

  fclose(entrada);
  return 0;
}
