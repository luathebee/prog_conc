/*  Programa de demonstração de uso de sockets UDP em C no Linux
 *  Funcionamento:
 * T2
 *
 *  O servidor envia outra palavra como resposta.
 *  Estado Atual: Com controle rudimentar de nível e temperatura
 *
	#---#----# TO DO #---#---#
	{
	{	melhoria da lógica de controle
	{	protreção de variáveis (quais???)
	{	registro de log com buffer duplo
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <pthread.h>

#define FALHA 1

#define	TAM_MEU_BUFFER	1000

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

/*CONSTANTES DO PROGRAMA*/
#define B 4
#define P 1000
#define S 4184
#define R 000.1
#define KpT 15
#define KpH 0.5
#define offset 0.8

int socket_local, iteractT = 0, iteractH = 0;

struct sockaddr_in endereco_destino;

float Q, Qi, Qe, Ni, Na, eT, Ut, Ta, T, Tref, H, Uh, Href, No;

char msg_recebidaT[1000],strMsgT[20], msg_recebidaH[1000], strMsgH[20];

pthread_mutex_t controlLock;

/*configs e funcoes de rede*/

int cria_socket_local(void)
{
	int socket_local;		/* Socket usado na comunicacão */

	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return 0;
	}
	return socket_local;
}


struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino)
{
	struct sockaddr_in servidor; 	/* Endereço do servidor incluindo ip e porta */
	struct hostent *dest_internet;	/* Endereço destino em formato próprio */
	struct in_addr dest_ip;		/* Endereço destino em formato ip numérico */

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}


void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem)
{
	/* Envia msg ao servidor */

	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 )
	{
		perror("sendto");
		return;
	}
}


int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER)
{
	int bytes_recebidos;		/* Número de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, TAM_BUFFER, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}


/*FUNÇÃO GETVALOR*/
float getValor(char msg[], int socket_local, struct sockaddr_in endereco_destino){

			int i, porta_destino = 12345;

			char msg_recebida[1000];

			envia_mensagem(socket_local, endereco_destino, msg);
			int nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
			for(i = 0; i <= 4; i++){
				msg_recebida[i] = msg_recebida[i+3];
			}
			msg_recebida[5] = '\0';
			//printf(msg_recebida);
			//printf("\n");
			return atof(msg_recebida);

}


/* MANDA VALOR, QUEBRADASSO, problema com ponteiros e retorno*/
/*int mandaValor(char tipo[], float valor, int socket_local, struct sockaddr_in endereco_destino){

			int porta_destino = 12345;
			char strValor[20], strMsg[20], msg_recebida[50];

			sprintf(strValor, "%f",valor);
			strValor[5] = '\0';
			strcpy(strMsg, tipo);
			strcat(strMsg, strValor);
			//printf("  <-Mensagem enviada\n");
			envia_mensagem(socket_local, endereco_destino, strMsg);
			int nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
			msg_recebida[ nrec ] = '\0';
			return &strMsg;

}*/

void terminal(){


}


				/*---------------	CONTROLA  ------------------/
				/----------------- TEMPERATURA ----------------*/
void controlaTemperatura(){
	int temporizador, i = 0, nrec;
	char msg_recebidaT[1000], strValor[20], strMsgT[20];

	//PEGA VALOR DA TEMPERATURA AMBIENTE

	pthread_mutex_lock(&controlLock);
	Ta = getValor("sta0", socket_local, endereco_destino);
	printf("	Temperatura ambiente em  %.1f\n",Ta);

	T = getValor("st-0", socket_local, endereco_destino);
	printf("	Temperatura atual em  %.1f\n",T);
	pthread_mutex_unlock(&controlLock);

	//DEFINE KE
	//float ke = (B * P) / No;

	//dados = fopen("arquivodados.txt", "w");
	//fprintf(dados, " 200 Medidas de altura H\n");

	while(1){
		//clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		iteractT++;

		/* do the stuff */
		pthread_mutex_unlock(&controlLock);
			//LEITURA DE VALORES
				T = getValor("st-0", socket_local, endereco_destino);
				Ta = getValor("sta0", socket_local, endereco_destino);

			//CALCULO DA AÇÃO DE CONTROLE
				eT = Tref - T;
				Ut = 100 * KpT * eT;
				Q = Q + Ut;

			//saturador
				if (Q < 0){ Q = 0;}
				if (Q > 1000000){ Q = 1000000;}

			//Ni = Ni*offset + Ut;

			//ENVIO DE VALORES
			/*  strMsg = mandaValor("ani", Ni, socket_local, endereco_destino);*/
				sprintf(strValor, "%f", Q);
				strValor[8] = '\0';
				strcpy(strMsgT, "aq-");
				strcat(strMsgT, strValor);
				envia_mensagem(socket_local, endereco_destino, strMsgT);
				nrec = recebe_mensagem(socket_local, msg_recebidaT, 1000);
				msg_recebidaT[ nrec ] = '\0';

		pthread_mutex_unlock(&controlLock);
		/*--Aquisição de tempo--
		//clock_gettime(CLOCK_MONOTONIC ,&tfin);
		//float varT = tfin.tv_nsec - t.tv_nsec;
		//printf(" Tempo = %.0f\n",varT); */

	 /*--printer--*/
	 if (iteractT%100 == 0){
		 	printf ("\n**********TEMPERATURA**********\n");
			printf("Iteração: %d\n",iteractT);
		    printf("Controle = %f \n",Ut);
			printf("%s  <-Mensagem enviada\n", strMsgT);
			printf("Mensagem de resposta com %d bytes >>>%s\n", nrec, msg_recebidaT);
	 }
		/*temporizador simples*/
		temporizador = 0;
		while(temporizador<5000000){ temporizador++; }



		///* calculate next shot */
		//clock_gettime(CLOCK_MONOTONIC ,&t);
        //t.tv_nsec += interval;
        //while (t.tv_nsec >= NSEC_PER_SEC) {
                //t.tv_nsec -= NSEC_PER_SEC;
                //t.tv_sec++;
        //}

	}

}

						/*---------------	CONTROLE  ------------------/
/						-----------------    NIVEL    ----------------*/
void controleNivel(){
		int temporizador, i = 0, nrec;
		char strValor[20];

		pthread_mutex_lock(&controlLock);
		No = getValor("sno0", socket_local, endereco_destino);
		pthread_mutex_unlock(&controlLock);

		printf("	Fluxo de saída de agua em %.1f\n",No);
		//valor inicial de Ni = No;
		Ni = No;
		float ke = (B * P) / No;

		/* do the stuff */
		while(1){

			iteractH++;

			pthread_mutex_lock(&controlLock);
			//LEITURA DE VALOR
				H = getValor("sh-0", socket_local, endereco_destino);

			//CALCULO DA AÇÃO DE CONTROLE
				Uh = ke * KpH * (Href - H);
				Ni = Ni*offset + Uh;
				if (Ni < 0){ Ni = 0;}
				if (Ni > 100){ Ni = 100;}
				//Ni = Ni*offset + Uh;

			//ENVIO DE VALOR
			/* strMsg = mandaValor("ani", Ni, socket_local, endereco_destino);*/
				sprintf(strValor, "%f",Ni);
				strValor[5] = '\0';
				strcpy(strMsgH, "ani");
				strcat(strMsgH, strValor);
				envia_mensagem(socket_local, endereco_destino, strMsgH);
				nrec = recebe_mensagem(socket_local, msg_recebidaH, 1000);
				msg_recebidaH[ nrec ] = '\0';

			pthread_mutex_unlock(&controlLock);
		 	/*--printer--*/
		 	if (iteractH%200 == 0){
				printf ("\n--------NÍVEL--------\n");
				printf("Iteração: %d\n",iteractH);
			   	printf("Controle = %f \n",Uh);
				printf("%s  <-Mensagem enviada\n", strMsgH);
				printf("Mensagem de resposta com %d bytes >>>%s\n", nrec, msg_recebidaH);
		 	}
			/*temporizador simples*/
			int temporizador = 0;
			while(temporizador<7000000){ temporizador++; }
		}
}


//--------------------------------------------------------------------------
//--------------------------------------MAIN--------------------------------
//--------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	if (argc != 5) {
		fprintf(stderr,"Uso: ctrlTemp endereço porta Tref Href \n");
		fprintf(stderr,"onde o endereço é o endereço do servidor \n");
		fprintf(stderr,"porta é o número da porta do servidor \n");
		fprintf(stderr,"Tref é a referencia de temperatura em ºC \n");
		fprintf(stderr,"Href é a referencia de nivel em m \n");
		fprintf(stderr,"exemplo de uso:\n");
		fprintf(stderr,"   udpcliente baker.das.ufsc.br 1234 30 2.5 \n");
		exit(FALHA);
	}


	/*config de rede*/
	int porta_destino = atoi( argv[2]);
	//pega porta, converte pra inteiro, salva na variavel

	socket_local = cria_socket_local();

	endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	FILE *dados;

	pthread_t threadTemp, threadNivel;

	Tref = atof (argv[3]);
	printf("	Referencia de Temperatura = %.1f\n",Tref);

	Href = atof (argv[4]);
	printf("	Referencia de Altura = %.1f\n",Href);

	pthread_create(&threadTemp, NULL, (void *)controlaTemperatura, NULL);
	pthread_create(&threadTemp, NULL, (void *)controleNivel, NULL);
	pthread_join(threadTemp, NULL);

}
