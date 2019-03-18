

OPENCV_INSTALL_DIR = /home/diego/install/opencv/

export LD_LIBRARY_PATH=$(OPENCV_INSTALL_DIR)/lib:/home/diego/install/script/lib

#Compilar programa compRoc
RocAll:
	#rm compRoc
	gcc compRoc.cpp -o compRoc -D__OPENCV__ -I$(OPENCV_INSTALL_DIR)/include/ -L$(OPENCV_INSTALL_DIR)/lib -lopencv_highgui -lopencv_core -lstdc++ -lopencv_imgproc -lm -I/home/diego/script/include -L/home/diego/script/lib -lvisiongl -I/opt/AMDAPP/include

#Compilar programa de extração
compMetodoFuzzy:
	#rm metodoFuzzy
	g++ metodoFuzzy.cpp -o metodoFuzzy -D__OPENCV__ -I$(OPENCV_INSTALL_DIR)/include/ -L$(OPENCV_INSTALL_DIR)/lib -lopencv_highgui -lopencv_core -lopencv_imgproc -lm -I/home/diego/script/include -L/home/diego/script/lib -lvisiongl -I/opt/AMDAPP/include

runAll:
	 for number in 9; do \
	   for tam in 3; do \
	     for bh in 2; do \
	       for jan1 in 3; do \
	  	  for val1 in 1; do \
		     for ts in 3.0; do \
			  ./metodoFuzzy $$number "Entradas/entradaDRIVE.txt" $$tam $$bh $$jan1 $$val1 $$ts;  \
			  ./compRoc "Diretorios/diretorioDRIVE.txt" $$number $$tam $$bh $$jan1 $$val1 $$ts "Resultados/listaDRIVE.txt"; \
	      	     done \
	           done \
	       done \
	     done \
	   done \
	 done

